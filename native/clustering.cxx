#include <jubatus/core/clustering/clustering.hpp>
#include "lib.hpp"
#include "helper.hpp"

int ClusteringInit(ClusteringObject *self, PyObject *args, PyObject *kwargs)
{
    std::string method;
    jubafvconv::converter_config fvconv_config;
    jubacomm::jsonconfig::config config;
    if (!ParseInitArgs(self, args, method, fvconv_config, config))
        return -1;
    try {
        std::string my_id;
        jubacore::clustering::clustering_config cluster_conf =
            jubacomm::jsonconfig::config_cast_check<
                jubacore::clustering::clustering_config>(config);
        self->handle.reset(
            new jubadriver::clustering(
                shared_ptr<jubacore::clustering::clustering>(
                    new jubacore::clustering::clustering(my_id, method, cluster_conf)),
                jubafvconv::make_fv_converter(fvconv_config, NULL)));
    } catch (std::exception &e) {
        PyErr_SetString(PyExc_TypeError, e.what());
        return -1;
    }
    return 0;
}

void ClusteringDealloc(ClusteringObject *self)
{
    self->handle.reset();
    self->config.reset();
    Py_TYPE(self)->tp_free((PyObject*)self);

}
