#include <Python.h>
#include <jubatus/core/common/jsonconfig.hpp>
#include <jubatus/core/framework/stream_writer.hpp>
#include <jubatus/core/fv_converter/datum_to_fv_converter.hpp>
#include <jubatus/core/storage/storage_factory.hpp>
#include <jubatus/core/anomaly/anomaly_factory.hpp>
#include <jubatus/core/driver/anomaly.hpp>
#include "lib.hpp"

#ifdef IS_PY3
static PyTypeObject *IdWithScoreType = NULL;
#else
static PyClassObject *IdWithScoreType = NULL;
#endif

int AnomalyInit(AnomalyObject *self, PyObject *args, PyObject *kwargs)
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
        std::string my_id;
        self->handle = shared_ptr<jubadriver::anomaly>
            (new jubadriver::anomaly(jubacore::anomaly::anomaly_factory::create_anomaly
                                     (method_value->get(), param, my_id),
                                     jubafvconv::make_fv_converter(converter_conf, NULL)));
        self->config = shared_ptr<std::string>(new std::string(str_config_json));
        self->idgen = 0;
    } catch (std::exception &e) {
        PyErr_SetString(PyExc_TypeError, e.what());
        return -1;
    } catch (...) {
        PyErr_SetString(PyExc_TypeError, "invalid config");
        return -1;
    }

    if (!IdWithScoreType) {
        PyObject *m = PyImport_ImportModule("jubatus.anomaly.types");
        if (m) {
#ifdef IS_PY3
            IdWithScoreType = (PyTypeObject*)PyObject_GetAttrString(m, "IdWithScore");
#else
            IdWithScoreType = (PyClassObject*)PyObject_GetAttrString(m, "IdWithScore");
#endif
            Py_DECREF(m);
        }
    }

    return 0;
}

void AnomalyDealloc(AnomalyObject *self)
{
    self->handle.reset();
    self->config.reset();
    Py_TYPE(self)->tp_free((PyObject*)self);
}

PyObject *AnomalyAdd(AnomalyObject *self, PyObject *py_datum)
{
    jubafvconv::datum datum;
    if (!PyDatumToNativeDatum(py_datum, datum))
        return NULL;
    std::string id_str = jubalang::lexical_cast<std::string>(self->idgen++);
    auto ret = self->handle->add(id_str, datum);
    PyObject *args = PyTuple_New(2);
    PyTuple_SetItem(args, 0, PyUnicode_DecodeUTF8(ret.first.c_str(), ret.first.size(), NULL));
    PyTuple_SetItem(args, 1, PyFloat_FromDouble(ret.second));
#ifdef IS_PY3
    PyObject *id_with_score = IdWithScoreType->tp_new(IdWithScoreType, args, NULL);
    IdWithScoreType->tp_init(id_with_score, args, NULL);
#else
    PyObject *id_with_score = PyInstance_New((PyObject*)IdWithScoreType, args, NULL);
#endif
    return id_with_score;
}

PyObject *AnomalyCalcScore(AnomalyObject *self, PyObject *args)
{
    jubafvconv::datum datum;
    if (!PyDatumToNativeDatum(args, datum))
        return NULL;
    return PyFloat_FromDouble(self->handle->calc_score(datum));
}
