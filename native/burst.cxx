#include "lib.hpp"
#include "helper.hpp"

int BurstInit(BurstObject *self, PyObject *args, PyObject *kwargs)
{
    std::string method;
    jubacomm::jsonconfig::config config;
    if (!ParseInitArgsWithoutConv(self, args, method, config))
        return -1;
    try {
        jubacore::burst::burst_options options =
            jubacomm::jsonconfig::config_cast_check<jubacore::burst::burst_options>(config);
        self->handle.reset(
            new jubadriver::burst(new jubacore::burst::burst(options)));
    } catch (std::exception &e) {
        PyErr_SetString(PyExc_TypeError, e.what());
        return -1;
    }
    return 0;
}

void BurstDealloc(BurstObject *self)
{
    self->handle.reset();
    self->config.reset();
    Py_TYPE(self)->tp_free((PyObject*)self);
}
