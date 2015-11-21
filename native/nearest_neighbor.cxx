#include <jubatus/core/storage/column_table.hpp>
#include <jubatus/core/nearest_neighbor/nearest_neighbor_factory.hpp>
#include "lib.hpp"

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
    return 0;
}

void NearestNeighborDealloc(NearestNeighborObject *self)
{
    self->handle.reset();
    self->config.reset();
    Py_TYPE(self)->tp_free((PyObject*)self);
}
