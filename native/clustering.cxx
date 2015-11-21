#include <jubatus/core/clustering/clustering.hpp>
#include "lib.hpp"
#include "helper.hpp"

static PyTypeObject *WeightedDatumType = NULL;

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
    if (!LookupTypeObject("jubatus.clustering.types", "WeightedDatum", &WeightedDatumType))
        return -1;
    return 0;
}

void ClusteringDealloc(ClusteringObject *self)
{
    self->handle.reset();
    self->config.reset();
    Py_TYPE(self)->tp_free((PyObject*)self);
}

PyObject *ClusteringPush(ClusteringObject *self, PyObject *args)
{
    if (!PyList_Check(args))
        return NULL;
    std::vector<jubafvconv::datum> points;
    for (Py_ssize_t i = 0; i < PyList_Size(args); ++i) {
        jubafvconv::datum d;
        PyObject *datum = PyList_GetItem(args, i);
        if (!PyDatumToNativeDatum(datum, d)) {
            return NULL;
        }
        points.push_back(d);
    }
    self->handle->push(points);
    Py_RETURN_TRUE;
}

static PyObject *Convert(const jubacore::clustering::cluster_unit &members)
{
    PyObject *out = PyList_New(members.size());
    for (int i = 0; i < members.size(); ++i) {
        PyObject *args = PyTuple_New(2);
        PyTuple_SetItem(args, 0, PyFloat_FromDouble(members[i].first));
        PyTuple_SetItem(args, 1, NativeDatumToPyDatum(members[i].second));
        PyList_SetItem(out, i, CreateInstanceAndInit(WeightedDatumType, args, NULL));
    }
    return out;
}

PyObject *ClusteringGetRevision(ClusteringObject *self, PyObject *args)
{
    return PyLong_FromLong(self->handle->get_revision());
}

PyObject *ClusteringGetCoreMembers(ClusteringObject *self, PyObject *args)
{
    jubacore::clustering::cluster_set clusters = self->handle->get_core_members();
    PyObject *out = PyList_New(clusters.size());
    for (int i = 0; i < clusters.size(); ++i) {
        PyList_SetItem(out, i, Convert(clusters[i]));
    }
    return out;
}

PyObject *ClusteringGetKCenter(ClusteringObject *self, PyObject *args)
{
    std::vector<jubafvconv::datum> points = self->handle->get_k_center();
    PyObject *out = PyList_New(points.size());
    for (int i = 0; i < points.size(); ++i) {
        PyList_SetItem(out, i, NativeDatumToPyDatum(points[i]));
    }
    return out;
}

PyObject *ClusteringGetNearestCenter(ClusteringObject *self, PyObject *args)
{
    jubafvconv::datum point;
    if (!PyDatumToNativeDatum(args, point))
        return NULL;
    return NativeDatumToPyDatum(self->handle->get_nearest_center(point));
}

PyObject *ClusteringGetNearestMembers(ClusteringObject *self, PyObject *args)
{
    jubafvconv::datum point;
    if (!PyDatumToNativeDatum(args, point))
        return NULL;
    return Convert(self->handle->get_nearest_members(point));
}
