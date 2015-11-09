#include <Python.h>
#include <jubatus/core/common/jsonconfig.hpp>
#include <jubatus/core/fv_converter/datum_to_fv_converter.hpp>
#include <jubatus/core/storage/storage_factory.hpp>
#include <jubatus/core/classifier/classifier_factory.hpp>
#include "lib.hpp"

#ifdef IS_PY3
static PyTypeObject *EstimateResultType = NULL;
#else
static PyClassObject *EstimateResultType = NULL;
#endif

int ClassifierInit(ClassifierObject *self, PyObject *args, PyObject *kwargs)
{
    PyObject *py_config_obj;
    if (!PyArg_ParseTuple(args, "O!", &PyDict_Type, &py_config_obj))
        return 0;
    std::string str_config_json;
    if (!PyDictToJson(py_config_obj, str_config_json))
        return 0;

    jubatus::util::text::json::json config_json =
        jubatus::util::lang::lexical_cast<jubatus::util::text::json::json>(str_config_json);
    jubatus::util::text::json::json_string *method_value =
        (jubatus::util::text::json::json_string*)config_json["method"].get();
    if (!method_value || method_value->type() != jubatus::util::text::json::json::String)
        return 0;

    jubatus::core::fv_converter::converter_config converter_conf;
    jubatus::util::text::json::from_json(config_json["converter"], converter_conf);
    jubatus::core::common::jsonconfig::config param(config_json["parameter"]);
    self->handle = jubatus::core::classifier::classifier_factory::create_classifier
        (method_value->get(), param, jubatus::core::storage::storage_factory::create_storage("local"));
    self->converter = jubatus::core::fv_converter::make_fv_converter(converter_conf, NULL);

    if (!EstimateResultType) {
        PyObject *m = PyImport_ImportModule("jubatus.classifier.types");
        if (m) {
#ifdef IS_PY3
            EstimateResultType = (PyTypeObject*)PyObject_GetAttrString(m, "EstimateResult");
#else
            EstimateResultType = (PyClassObject*)PyObject_GetAttrString(m, "EstimateResult");
#endif
            Py_DECREF(m);
        }
    }
    return 0;
}

void ClassifierDealloc(ClassifierObject *self)
{
    self->handle.reset();
    self->converter.reset();
    Py_TYPE(self)->tp_free((PyObject*)self);
}

PyObject *ClassifierTrain(ClassifierObject *self, PyObject *args)
{
    PyObject *list;
    std::string str;
    jubatus::core::fv_converter::datum d;
    jubatus::core::common::sfv_t fv;
    if (!PyArg_ParseTuple(args, "O!", &PyList_Type, &list))
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
        self->converter->convert(d, fv);
        self->handle->train(fv, str);
    }
    Py_RETURN_NONE;
}

PyObject *ClassifierClassify(ClassifierObject *self, PyObject *args)
{
    PyObject *list;
    jubatus::core::fv_converter::datum d;
    jubatus::core::common::sfv_t fv;
    jubatus::core::classifier::classify_result ret;
    if (!PyArg_ParseTuple(args, "O!", &PyList_Type, &list))
        return NULL;
    PyObject *out = PyList_New(PyList_Size(list));
    for (Py_ssize_t i = 0; i < PyList_Size(list); ++i) {
        PyObject *datum = PyList_GetItem(list, i);
        if (!PyDatumToNativeDatum(datum, d)) {
            Py_DECREF(out);
            return NULL;
        }
        self->converter->convert(d, fv);
        self->handle->classify_with_scores(fv, ret);
        PyObject *tmp = PyList_New(ret.size());
        for (int j = 0; j < ret.size(); ++j) {
            PyObject *args = PyTuple_New(2);
            PyTuple_SetItem(args, 0, PyUnicode_DecodeUTF8(ret[j].label.c_str(), ret[j].label.size(), NULL));
            PyTuple_SetItem(args, 1, PyFloat_FromDouble(ret[j].score));
#ifdef IS_PY3
            PyObject *er = EstimateResultType->tp_new(EstimateResultType, args, NULL);
            EstimateResultType->tp_init(er, args, NULL);
#else
            PyObject *er = PyInstance_New((PyObject*)EstimateResultType, args, NULL);
#endif            
            PyList_SetItem(tmp, j, er);
        }
        PyList_SetItem(out, i, tmp);
    }
    return out;
}
