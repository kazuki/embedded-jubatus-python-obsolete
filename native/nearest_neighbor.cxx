#include <jubatus/core/storage/column_table.hpp>
#include <jubatus/core/nearest_neighbor/nearest_neighbor_factory.hpp>
#include "lib.hpp"
#include "helper.hpp"

static PyTypeObject *IdWithScoreType = NULL;
static const std::string TYPE("nearest_neighbor");
static const uint64_t USER_DATA_VERSION = 1;

int NearestNeighborInit(NearestNeighborObject *self, PyObject *args, PyObject *kwargs)
{
    std::string method;
    jubafvconv::converter_config fvconv_config;
    jubacomm::jsonconfig::config config;
    if (!ParseInitArgs(self, args, method, fvconv_config, config))
        return -1;
    try {
        shared_ptr<jubacore::storage::column_table> table(new jubacore::storage::column_table);
        std::string my_id;
        self->handle.reset(
            new jubadriver::nearest_neighbor(
                jubacore::nearest_neighbor::create_nearest_neighbor(
                    method, config, table, my_id),
                jubafvconv::make_fv_converter(fvconv_config, NULL)));
    } catch (std::exception &e) {
        PyErr_SetString(PyExc_TypeError, e.what());
        return -1;
    }
    if (!LookupTypeObject("jubatus.nearest_neighbor.types", "IdWithScore", &IdWithScoreType))
        return -1;
    return 0;
}

void NearestNeighborDealloc(NearestNeighborObject *self)
{
    self->handle.reset();
    self->config.reset();
    Py_TYPE(self)->tp_free((PyObject*)self);
}

PyObject *NearestNeighborSetRow(NearestNeighborObject *self, PyObject *args)
{
    std::string id;
    jubafvconv::datum datum;
    if (!ParseArgument(args, id, datum))
        return NULL;
    CATCH_CPP_EXCEPTION_AND_RETURN_NULL(self->handle->set_row(id, datum));
    Py_RETURN_TRUE;
}

PyObject *NearestNeighborNeighborRowFromId(NearestNeighborObject *self, PyObject *args)
{
    std::string id;
    long size;
    if (!ParseArgument(args, id, size))
        return NULL;
    return ConvertToIdWithScoreList(
        self->handle->neighbor_row_from_id(id, size),
        IdWithScoreType);
}
PyObject *NearestNeighborNeighborRowFromDatum(NearestNeighborObject *self, PyObject *args)
{
    jubafvconv::datum d;
    long size;
    if (!ParseArgument(args, d, size))
        return NULL;
    std::vector<std::pair<std::string, float> > list;
    CATCH_CPP_EXCEPTION_AND_RETURN_NULL(list = self->handle->neighbor_row_from_datum(d, size));
    return ConvertToIdWithScoreList(list, IdWithScoreType);
}

PyObject *NearestNeighborSimilarRowFromId(NearestNeighborObject *self, PyObject *args)
{
    std::string id;
    long size;
    if (!ParseArgument(args, id, size))
        return NULL;
    std::vector<std::pair<std::string, float> > list;
    CATCH_CPP_EXCEPTION_AND_RETURN_NULL(list = self->handle->similar_row(id, size));
    return ConvertToIdWithScoreList(list, IdWithScoreType);
}

PyObject *NearestNeighborSimilarRowFromDatum(NearestNeighborObject *self, PyObject *args)
{
    jubafvconv::datum d;
    long size;
    if (!ParseArgument(args, d, size))
        return NULL;
    std::vector<std::pair<std::string, float> > list;
    CATCH_CPP_EXCEPTION_AND_RETURN_NULL(list = self->handle->similar_row(d, size));
    return ConvertToIdWithScoreList(list, IdWithScoreType);
}

PyObject *NearestNeighborGetAllRows(NearestNeighborObject *self, PyObject *args)
{
    std::vector<std::string> rows;
    CATCH_CPP_EXCEPTION_AND_RETURN_NULL(rows = self->handle->get_all_rows());
    return Convert(rows);
}

PyObject *NearestNeighborDump(NearestNeighborObject *self, PyObject *args)
{
    return CommonApiDump(self, TYPE, USER_DATA_VERSION);
}

PyObject *NearestNeighborLoad(NearestNeighborObject *self, PyObject *args)
{
    return CommonApiLoad(self, args, TYPE, USER_DATA_VERSION);
}
