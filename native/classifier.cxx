#include <jubatus/core/storage/storage_factory.hpp>
#include <jubatus/core/classifier/classifier_factory.hpp>
#include "lib.hpp"
#include "helper.hpp"

static PyTypeObject *EstimateResultType = NULL;
static const std::string TYPE("classifier");
static const uint64_t USER_DATA_VERSION = 1;

int ClassifierInit(ClassifierObject *self, PyObject *args, PyObject *kwargs)
{
    std::string method;
    jubafvconv::converter_config fvconv_config;
    jubacomm::jsonconfig::config config;
    if (!ParseInitArgs(self, args, method, fvconv_config, config))
        return -1;
    try {
        self->handle.reset(
            new jubadriver::classifier(
                jubacore::classifier::classifier_factory::create_classifier(
                    method, config,
                    jubacore::storage::storage_factory::create_storage("local")),
                jubafvconv::make_fv_converter(fvconv_config, NULL)));
    } catch (std::exception &e) {
        PyErr_SetString(PyExc_TypeError, e.what());
        return -1;
    }
    if (!LookupTypeObject("jubatus.classifier.types", "EstimateResult", &EstimateResultType))
        return -1;
    return 0;
}

void ClassifierDealloc(ClassifierObject *self)
{
    self->handle.reset();
    self->config.reset();
    Py_TYPE(self)->tp_free((PyObject*)self);
}

PyObject *ClassifierTrain(ClassifierObject *self, PyObject *list)
{
    std::string str;
    jubafvconv::datum d;
    if (!PyList_Check(list))
        return NULL;
    for (Py_ssize_t i = 0; i < PyList_Size(list); ++i) {
        PyObject *labeled_datum = PyList_GetItem(list, i);
        if (!labeled_datum)
            return NULL;
        PyObject *label = PyTuple_GetItem(labeled_datum, 0);
        if (!label)
            return NULL;
        PyObject *datum = PyTuple_GetItem(labeled_datum, 1);
        if (!datum)
            return NULL;
        if (!PyUnicodeToUTF8(label, str))
            return NULL;

        PyDatumToNativeDatum(datum, d);
        CATCH_CPP_EXCEPTION_AND_RETURN_NULL(self->handle->train(str, d));
    }
    Py_RETURN_NONE;
}

PyObject *ClassifierClassify(ClassifierObject *self, PyObject *list)
{
    jubafvconv::datum d;
    if (!PyList_Check(list))
        return NULL;
    PyObject *out = PyList_New(PyList_Size(list));
    for (Py_ssize_t i = 0; i < PyList_Size(list); ++i) {
        PyObject *datum = PyList_GetItem(list, i);
        if (!PyDatumToNativeDatum(datum, d)) {
            Py_DECREF(out);
            return NULL;
        }
        jubacore::classifier::classify_result ret;
        CATCH_CPP_EXCEPTION_AND_RETURN_NULL2(ret = self->handle->classify(d),
                                             Py_DECREF(out));
        PyObject *tmp = PyList_New(ret.size());
        for (int j = 0; j < ret.size(); ++j) {
            PyObject *args = PyTuple_New(2);
            PyTuple_SetItem(args, 0, PyUnicode_DecodeUTF8_FromString(ret[j].label));
            PyTuple_SetItem(args, 1, PyFloat_FromDouble(ret[j].score));
            PyList_SetItem(tmp, j, CreateInstanceAndInit(EstimateResultType, args, NULL));
        }
        PyList_SetItem(out, i, tmp);
    }
    return out;
}

PyObject *ClassifierGetLabels(ClassifierObject *self, PyObject*)
{
    std::vector<std::string> labels;
    CATCH_CPP_EXCEPTION_AND_RETURN_NULL(labels = self->handle->get_labels());
    return Convert(labels);
}

PyObject *ClassifierSetLabel(ClassifierObject *self, PyObject *args)
{
    std::string label;
    if (!PyUnicodeToUTF8(args, label))
        return NULL;
    CATCH_CPP_EXCEPTION_AND_RETURN_NULL(self->handle->set_label(label));
    Py_RETURN_NONE;
}

PyObject *ClassifierDeleteLabel(ClassifierObject *self, PyObject *args)
{
    std::string label;
    if (!PyUnicodeToUTF8(args, label))
        return NULL;
    CATCH_CPP_EXCEPTION_AND_RETURN_NULL(self->handle->delete_label(label));
    Py_RETURN_NONE;
}

PyObject *ClassifierDump(ClassifierObject *self, PyObject *args)
{
    return CommonApiDump(self, TYPE, USER_DATA_VERSION);
}

PyObject *ClassifierLoad(ClassifierObject *self, PyObject *args)
{
    return CommonApiLoad(self, args, TYPE, USER_DATA_VERSION);
}
