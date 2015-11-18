#include <Python.h>
#include <jubatus/core/common/jsonconfig.hpp>
#include <jubatus/core/framework/stream_writer.hpp>
#include <jubatus/core/fv_converter/datum_to_fv_converter.hpp>
#include <jubatus/core/storage/storage_factory.hpp>
#include <jubatus/core/classifier/classifier_factory.hpp>
#include <jubatus/core/driver/classifier.hpp>
#include "lib.hpp"

#ifdef IS_PY3
static PyTypeObject *EstimateResultType = NULL;
#else
static PyClassObject *EstimateResultType = NULL;
#endif

static const std::string TYPE("classifier");

int ClassifierInit(ClassifierObject *self, PyObject *args, PyObject *kwargs)
{
    PyObject *py_config_obj;
    if (!PyArg_ParseTuple(args, "O!", &PyDict_Type, &py_config_obj))
        return -1;
    std::string str_config_json;
    if (!PyDictToJson(py_config_obj, str_config_json))
        return -1;

    auto config_json = jubalang::lexical_cast<jubajson::json>(str_config_json);
    auto method_value = (jubajson::json_string*)config_json["method"].get();
    if (!method_value || method_value->type() != jubajson::json::String) {
        PyErr_SetString(PyExc_TypeError, "invalid config");
        return -1;
    }

    try {
        jubafvconv::converter_config converter_conf;
        jubajson::from_json(config_json["converter"], converter_conf);
        jubacomm::jsonconfig::config param(config_json["parameter"]);
        self->handle.reset(
            new jubadriver::classifier(
                jubacore::classifier::classifier_factory::create_classifier(
                    method_value->get(), param,
                    jubacore::storage::storage_factory::create_storage("local")),
                jubafvconv::make_fv_converter(converter_conf, NULL)));
        self->config.reset(new std::string(str_config_json));
    } catch (std::exception &e) {
        PyErr_SetString(PyExc_TypeError, e.what());
        return -1;
    } catch (...) {
        PyErr_SetString(PyExc_TypeError, "invalid config");
        return -1;
    }

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
        self->handle->train(str, d);
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
        jubacore::classifier::classify_result ret = self->handle->classify(d);
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

PyObject *ClassifierGetLabels(ClassifierObject *self, PyObject*)
{
    std::vector<std::string> labels = self->handle->get_labels();
    PyObject *ret = PyList_New(labels.size());
    for (int i = 0; i < labels.size(); ++i) {
        PyList_SetItem(ret, i, PyUnicode_DecodeUTF8(labels[i].c_str(), labels[i].size(), NULL));
    }
    return ret;
}

PyObject *ClassifierSetLabel(ClassifierObject *self, PyObject *args)
{
    std::string label;
    if (!PyUnicodeToUTF8(args, label))
        return NULL;
    self->handle->set_label(label);
    Py_RETURN_NONE;
}

PyObject *ClassifierDeleteLabel(ClassifierObject *self, PyObject *args)
{
    std::string label;
    if (!PyUnicodeToUTF8(args, label))
        return NULL;
    self->handle->delete_label(label);
    Py_RETURN_NONE;
}

PyObject *ClassifierDump(ClassifierObject *self, PyObject *args)
{
    static const std::string ID("");
    msgpack::sbuffer user_data_buf;
    {
        jubaframework::stream_writer<msgpack::sbuffer> st(user_data_buf);
        jubaframework::jubatus_packer jp(st);
        jubaframework::packer packer(jp);
        packer.pack_array(2);
        packer.pack((uint64_t)1); // Ref: jubatus/server/server/classifier_serv.cpp get_user_data_version
        self->handle->pack(packer);
    }
    return SerializeModel(TYPE, *self->config, ID, user_data_buf);
}

PyObject *ClassifierLoad(ClassifierObject *self, PyObject *args)
{
    msgpack::unpacked unpacked;
    uint64_t user_data_version;
    msgpack::object *user_data;
    std::string model_type, model_id, model_config;
    if (!LoadModelHelper(args, unpacked, model_type, model_id, model_config, &user_data_version, &user_data))
        return NULL;
    if (model_type != TYPE || *(self->config) != model_config)
        return NULL;
    self->handle->unpack(*user_data);
    Py_RETURN_NONE;
}

PyObject *ClassifierClear(ClassifierObject *self, PyObject *args)
{
    self->handle->clear();
    Py_RETURN_NONE;
}

PyObject *ClassifierGetConfig(ClassifierObject *self, PyObject *args)
{
    return PyUnicode_DecodeUTF8(self->config->c_str(), self->config->size(), NULL);
}
