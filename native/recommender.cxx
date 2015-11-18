#include <Python.h>
#include <jubatus/core/common/jsonconfig.hpp>
#include <jubatus/core/framework/stream_writer.hpp>
#include <jubatus/core/fv_converter/datum_to_fv_converter.hpp>
#include <jubatus/core/storage/storage_factory.hpp>
#include <jubatus/core/recommender/recommender_factory.hpp>
#include <jubatus/core/driver/recommender.hpp>
#include "lib.hpp"

#ifdef IS_PY3
static PyTypeObject *IdWithScoreType = NULL;
#else
static PyClassObject *IdWithScoreType = NULL;
#endif

int RecommenderInit(RecommenderObject *self, PyObject *args, PyObject *kwargs)
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
        self->handle.reset(
            new jubadriver::recommender(
                jubacore::recommender::recommender_factory::create_recommender(
                    method_value->get(), param, my_id),
               jubafvconv::make_fv_converter(converter_conf, NULL)));
        self->config.reset(new std::string(str_config_json));
    } catch (std::exception &e) {
        PyErr_SetString(PyExc_TypeError, e.what());
        return -1;
    } catch (...) {
        PyErr_SetString(PyExc_TypeError, "invalid config");
        return -1;
    }

    if (!IdWithScoreType) {
        PyObject *m = PyImport_ImportModule("jubatus.recommender.types");
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

void RecommenderDealloc(RecommenderObject *self)
{
    self->handle.reset();
    self->config.reset();
    Py_TYPE(self)->tp_free((PyObject*)self);
}
