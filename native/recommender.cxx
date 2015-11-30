#include <jubatus/core/recommender/recommender_factory.hpp>
#include "lib.hpp"
#include "helper.hpp"

static PyTypeObject *IdWithScoreType = NULL;
static const std::string TYPE("recommender");
static const uint64_t USER_DATA_VERSION = 1;

int RecommenderInit(RecommenderObject *self, PyObject *args, PyObject *kwargs)
{
    std::string method;
    jubafvconv::converter_config fvconv_config;
    jubacomm::jsonconfig::config config;
    if (!ParseInitArgs(self, args, method, fvconv_config, config))
        return -1;
    try {
        std::string my_id;
        self->handle.reset(
            new jubadriver::recommender(
                jubacore::recommender::recommender_factory::create_recommender(
                    method, config, my_id),
               jubafvconv::make_fv_converter(fvconv_config, NULL)));
    } catch (std::exception &e) {
        PyErr_SetString(PyExc_TypeError, e.what());
        return -1;
    }
    if (!LookupTypeObject("jubatus.recommender.types", "IdWithScore", &IdWithScoreType))
        return -1;
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
    CATCH_CPP_EXCEPTION_AND_RETURN_NULL(self->handle->clear_row(id));
    Py_RETURN_TRUE;
}

PyObject *RecommenderUpdateRow(RecommenderObject *self, PyObject *args)
{
    std::string id;
    jubafvconv::datum datum;
    if (!ParseArgument(args, id, datum))
        return NULL;
    CATCH_CPP_EXCEPTION_AND_RETURN_NULL(self->handle->update_row(id, datum));
    Py_RETURN_TRUE;
}

PyObject *RecommenderCompleteRowFromId(RecommenderObject *self, PyObject *args)
{
    std::string id;
    if (!PyUnicodeToUTF8(args, id))
        return NULL;
    jubafvconv::datum datum;
    CATCH_CPP_EXCEPTION_AND_RETURN_NULL(datum = self->handle->complete_row_from_id(id));
    return NativeDatumToPyDatum(datum);
}

PyObject *RecommenderCompleteRowFromDatum(RecommenderObject *self, PyObject *args)
{
    jubafvconv::datum datum;
    if (!PyDatumToNativeDatum(args, datum))
        return NULL;
    CATCH_CPP_EXCEPTION_AND_RETURN_NULL(datum = self->handle->complete_row_from_datum(datum));
    return NativeDatumToPyDatum(datum);
}

PyObject *RecommenderSimilarRowFromId(RecommenderObject *self, PyObject *args)
{
    std::string id;
    long size;
    if (!ParseArgument(args, id, size))
        return NULL;
    std::vector<std::pair<std::string, float> > list;
    CATCH_CPP_EXCEPTION_AND_RETURN_NULL(list = self->handle->similar_row_from_id(id, size));
    return ConvertToIdWithScoreList(list, IdWithScoreType);
}

PyObject *RecommenderSimilarRowFromDatum(RecommenderObject *self, PyObject *args)
{
    jubafvconv::datum datum;
    long size;
    if (!ParseArgument(args, datum, size))
        return NULL;
    std::vector<std::pair<std::string, float> > list;
    CATCH_CPP_EXCEPTION_AND_RETURN_NULL(list = self->handle->similar_row_from_datum(datum, size));
    return ConvertToIdWithScoreList(list, IdWithScoreType);
}

PyObject *RecommenderDecodeRow(RecommenderObject *self, PyObject *args)
{
    std::string id;
    if (!PyUnicodeToUTF8(args, id))
        return NULL;
    jubafvconv::datum d;
    CATCH_CPP_EXCEPTION_AND_RETURN_NULL(d = self->handle->decode_row(id));
    return NativeDatumToPyDatum(d);
}

PyObject *RecommenderGetAllRows(RecommenderObject *self, PyObject *args)
{
    std::vector<std::string> vec;
    CATCH_CPP_EXCEPTION_AND_RETURN_NULL(vec = self->handle->get_all_rows());
    PyObject *ret = PyList_New(vec.size());
    for (int i = 0; i < vec.size(); ++i) {
        PyList_SetItem(ret, i, PyUnicode_DecodeUTF8_FromString(vec[i]));
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
    double similarity;
    CATCH_CPP_EXCEPTION_AND_RETURN_NULL(similarity = self->handle->calc_similarity(datum1, datum2));
    return PyFloat_FromDouble(similarity);
}

PyObject *RecommenderCalcL2Norm(RecommenderObject *self, PyObject *args)
{
    jubafvconv::datum datum;
    if (!PyDatumToNativeDatum(args, datum))
        return NULL;
    double l2norm;
    CATCH_CPP_EXCEPTION_AND_RETURN_NULL(l2norm = self->handle->calc_l2norm(datum));
    return PyFloat_FromDouble(l2norm);
}

PyObject *RecommenderDump(RecommenderObject *self, PyObject *args)
{
    return CommonApiDump(self, TYPE, USER_DATA_VERSION);
}

PyObject *RecommenderLoad(RecommenderObject *self, PyObject *args)
{
    return CommonApiLoad(self, args, TYPE, USER_DATA_VERSION);
}
