#include <jubatus/core/storage/storage_factory.hpp>
#include <jubatus/core/regression/regression_factory.hpp>
#include "lib.hpp"

static PyTypeObject *ScoredDatumType = NULL;

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
    if (!LookupTypeObject("jubatus.regression.types", "ScoredDatum", &ScoredDatumType))
        return -1;
    return 0;
}

void RegressionDealloc(RegressionObject *self)
{
    self->handle.reset();
    self->config.reset();
    Py_TYPE(self)->tp_free((PyObject*)self);
}

PyObject *RegressionTrain(RegressionObject *self, PyObject *train_data)
{
    jubafvconv::datum d;
    if (!PyList_Check(train_data))
        return NULL;
    for (Py_ssize_t i = 0; i < PyList_Size(train_data); ++i) {
        PyObject *scored_datum = PyList_GetItem(train_data, i);
        PyObject *score = PyObject_GetAttrString(scored_datum, "score"); // ret: new-ref
        if (!score)
            return NULL;
        if (!PyFloat_Check(score)) {
            Py_DECREF(score);
            return NULL;
        }
        float score_value = (float)PyFloat_AsDouble(score);
        Py_DECREF(score);
        PyObject *datum = PyObject_GetAttrString(scored_datum, "data"); // ret: new-ref
        if (!datum)
            return NULL;
        if (!PyDatumToNativeDatum(datum, d)) {
            Py_DECREF(datum);
            return NULL;
        }
        self->handle->train(
            std::pair<float, jubafvconv::datum>(score_value, d));
    }
    return PyLong_FromLong(PyList_Size(train_data));
}

PyObject *RegressionEstimate(RegressionObject *self, PyObject *estimate_data_list)
{
    jubafvconv::datum d;
    if (!PyList_Check(estimate_data_list))
        return NULL;
    PyObject *out = PyList_New(PyList_Size(estimate_data_list));
    for (Py_ssize_t i = 0; i < PyList_Size(estimate_data_list); ++i) {
        PyObject *datum = PyList_GetItem(estimate_data_list, i);
        if (!PyDatumToNativeDatum(datum, d)) {
            Py_DECREF(out);
            return NULL;
        }
        PyList_SetItem(out, i, PyFloat_FromDouble(self->handle->estimate(d)));
    }
    return out;
}
