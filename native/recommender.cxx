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

    jubajson::json config_json = jubalang::lexical_cast<jubajson::json>(str_config_json);
    jubajson::json_string *method_value = (jubajson::json_string*)config_json["method"].get();
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

PyObject *RecommenderClearRow(RecommenderObject *self, PyObject *args)
{
    std::string id;
    if (!PyUnicodeToUTF8(args, id))
        return NULL;
    self->handle->clear_row(id);
    Py_RETURN_TRUE;
}

PyObject *RecommenderUpdateRow(RecommenderObject *self, PyObject *args)
{
    PyObject *py_id;
    PyObject *py_datum;
    if (!PyArg_UnpackTuple(args, "args", 2, 2, &py_id, &py_datum))
        return NULL;

    std::string id;
    jubafvconv::datum datum;
    if (!PyUnicodeToUTF8(py_id, id))
        return NULL;
    if (!PyDatumToNativeDatum(py_datum, datum))
        return NULL;

    self->handle->update_row(id, datum);
    Py_RETURN_TRUE;
}

PyObject *RecommenderCompleteRowFromId(RecommenderObject *self, PyObject *args)
{
    std::string id;
    if (!PyUnicodeToUTF8(args, id))
        return NULL;
    jubafvconv::datum datum = self->handle->complete_row_from_id(id);
    return NativeDatumToPyDatum(datum);
}

PyObject *RecommenderCompleteRowFromDatum(RecommenderObject *self, PyObject *args)
{
    jubafvconv::datum datum;
    if (!PyDatumToNativeDatum(args, datum))
        return NULL;
    datum = self->handle->complete_row_from_datum(datum);
    return NativeDatumToPyDatum(datum);
}

PyObject *RecommenderSimilarRowFromId(RecommenderObject *self, PyObject *args)
{
    PyObject *py_id;
    PyObject *py_size;
    if (!PyArg_UnpackTuple(args, "args", 2, 2, &py_id, &py_size))
        return NULL;

#ifdef IS_PY3
    if (!PyLong_Check(py_size))
        return NULL;
    long size = PyLong_AsLong(py_size);
#else
    if (!PyLong_Check(py_size) && !PyInt_Check(py_size))
        return NULL;
    long size = (PyInt_Check(py_size) ? PyInt_AsLong(py_size) : PyLong_AsLong(py_size));
#endif
    std::string id;
    if (!PyUnicodeToUTF8(py_id, id))
        return NULL;
    std::vector<std::pair<std::string, float> > ret = self->handle->similar_row_from_id(id, size);
    PyObject *vec = PyList_New(ret.size());
    for (int i = 0; i < ret.size(); ++i) {
        PyObject *args = PyTuple_New(2);
        PyTuple_SetItem(args, 0, PyUnicode_DecodeUTF8(ret[i].first.data(),
                                                      ret[i].first.size(), NULL));
        PyTuple_SetItem(args, 1, PyFloat_FromDouble(ret[i].second));
#ifdef IS_PY3
        PyObject *item = IdWithScoreType->tp_new(IdWithScoreType, args, NULL);
        IdWithScoreType->tp_init(item, args, NULL);
#else
        PyObject *item = PyInstance_New((PyObject*)IdWithScoreType, args, NULL);
#endif
        PyList_SetItem(vec, i, item);
    }
    return vec;
}

PyObject *RecommenderSimilarRowFromDatum(RecommenderObject *self, PyObject *args)
{
    PyObject *py_datum;
    PyObject *py_size;
    if (!PyArg_UnpackTuple(args, "args", 2, 2, &py_datum, &py_size))
        return NULL;

#ifdef IS_PY3
    if (!PyLong_Check(py_size))
        return NULL;
    long size = PyLong_AsLong(py_size);
#else
    if (!PyLong_Check(py_size) && !PyInt_Check(py_size))
        return NULL;
    long size = (PyInt_Check(py_size) ? PyInt_AsLong(py_size) : PyLong_AsLong(py_size));
#endif
    jubafvconv::datum datum;
    if (!PyDatumToNativeDatum(py_datum, datum))
        return NULL;
    std::vector<std::pair<std::string, float> > ret = self->handle->similar_row_from_datum(datum, size);
    PyObject *vec = PyList_New(ret.size());
    for (int i = 0; i < ret.size(); ++i) {
        PyObject *args = PyTuple_New(2);
        PyTuple_SetItem(args, 0, PyUnicode_DecodeUTF8(ret[i].first.data(),
                                                      ret[i].first.size(), NULL));
        PyTuple_SetItem(args, 1, PyFloat_FromDouble(ret[i].second));
#ifdef IS_PY3
        PyObject *item = IdWithScoreType->tp_new(IdWithScoreType, args, NULL);
        IdWithScoreType->tp_init(item, args, NULL);
#else
        PyObject *item = PyInstance_New((PyObject*)IdWithScoreType, args, NULL);
#endif
        PyList_SetItem(vec, i, item);
    }
    return vec;
}

PyObject *RecommenderDecodeRow(RecommenderObject *self, PyObject *args)
{
    std::string id;
    if (!PyUnicodeToUTF8(args, id))
        return NULL;
    return NativeDatumToPyDatum(self->handle->decode_row(id));
}

PyObject *RecommenderGetAllRows(RecommenderObject *self, PyObject *args)
{
    std::vector<std::string> vec = self->handle->get_all_rows();
    PyObject *ret = PyList_New(vec.size());
    for (int i = 0; i < vec.size(); ++i) {
        PyList_SetItem(ret, i, PyUnicode_DecodeUTF8(vec[i].data(),
                                                    vec[i].size(),
                                                    NULL));
    }
    return ret;
}

PyObject *RecommenderCalcSimilarity(RecommenderObject *self, PyObject *args)
{
    PyObject *py_datum1;
    PyObject *py_datum2;
    if (!PyArg_UnpackTuple(args, "args", 2, 2, &py_datum1, &py_datum2))
        return NULL;

    jubafvconv::datum datum1, datum2;
    if (!PyDatumToNativeDatum(py_datum1, datum1))
        return NULL;
    if (!PyDatumToNativeDatum(py_datum2, datum2))
        return NULL;
    return PyFloat_FromDouble(self->handle->calc_similality(datum1, datum2));
}

PyObject *RecommenderCalcL2Norm(RecommenderObject *self, PyObject *args)
{
    jubafvconv::datum datum;
    if (!PyDatumToNativeDatum(args, datum))
        return NULL;
    return PyFloat_FromDouble(self->handle->calc_l2norm(datum));
}
