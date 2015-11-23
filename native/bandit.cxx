#include "lib.hpp"
#include "helper.hpp"

static const std::string TYPE("bandit");
static const uint64_t USER_DATA_VERSION = 1;

int BanditInit(BanditObject *self, PyObject *args, PyObject *kwargs)
{
    std::string method;
    jubacomm::jsonconfig::config config;
    if (!ParseInitArgsWithoutConv(self, args, method, config))
        return -1;
    try {
        self->handle.reset(new jubadriver::bandit(method, config));
    } catch (std::exception &e) {
        PyErr_SetString(PyExc_TypeError, e.what());
        return -1;
    }
    return 0;
}

void BanditDealloc(BanditObject *self)
{
    self->handle.reset();
    self->config.reset();
    Py_TYPE(self)->tp_free((PyObject*)self);
}

PyObject *BanditDump(BanditObject *self, PyObject *args)
{
    return CommonApiDump(self, TYPE, USER_DATA_VERSION);
}

PyObject *BanditLoad(BanditObject *self, PyObject *args)
{
    return CommonApiLoad(self, args, TYPE, USER_DATA_VERSION);
}
