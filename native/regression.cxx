#include <jubatus/core/storage/storage_factory.hpp>
#include <jubatus/core/regression/regression_factory.hpp>
#include "lib.hpp"

int RegressionInit(RegressionObject *self, PyObject *args, PyObject *kwargs)
{
    std::string method;
    jubafvconv::converter_config fvconv_config;
    jubacomm::jsonconfig::config config;
    if (!ParseInitArgs(self, args, method, fvconv_config, config))
        return -1;
    try {
        shared_ptr<jubacore::storage::storage_base> model =
            jubacore::storage::storage_factory::create_storage("local");
        self->handle.reset(
            new jubadriver::regression(
                model,
                jubacore::regression::regression_factory::create_regression(
                    method, config, model),
                jubafvconv::make_fv_converter(fvconv_config, NULL)));
    } catch (std::exception &e) {
        PyErr_SetString(PyExc_TypeError, e.what());
        return -1;
    }
    return 0;
}

void RegressionDealloc(RegressionObject *self)
{
    self->handle.reset();
    self->config.reset();
    Py_TYPE(self)->tp_free((PyObject*)self);
}
