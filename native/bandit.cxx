#include "lib.hpp"

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
