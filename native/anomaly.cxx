#include <jubatus/core/anomaly/anomaly_factory.hpp>
#include "lib.hpp"
#include "helper.hpp"

static PyTypeObject *IdWithScoreType = NULL;

int AnomalyInit(AnomalyObject *self, PyObject *args, PyObject *kwargs)
{
    std::string method;
    jubafvconv::converter_config fvconv_config;
    jubacomm::jsonconfig::config config;
    if (!ParseInitArgs(self, args, method, fvconv_config, config))
        return -1;
    try {
        std::string my_id;
        self->handle.reset(
            new jubadriver::anomaly(
                jubacore::anomaly::anomaly_factory::create_anomaly(
                    method, config, my_id),
               jubafvconv::make_fv_converter(fvconv_config, NULL)));
        self->idgen = 0;
    } catch (std::exception &e) {
        PyErr_SetString(PyExc_TypeError, e.what());
        return -1;
    }
    if (!LookupTypeObject("jubatus.anomaly.types", "IdWithScore", &IdWithScoreType))
        return -1;
    return 0;
}

void AnomalyDealloc(AnomalyObject *self)
{
    self->handle.reset();
    self->config.reset();
    Py_TYPE(self)->tp_free((PyObject*)self);
}

PyObject *AnomalyAdd(AnomalyObject *self, PyObject *py_datum)
{
    jubafvconv::datum datum;
    if (!PyDatumToNativeDatum(py_datum, datum))
        return NULL;
    std::string id_str = jubalang::lexical_cast<std::string>(self->idgen++);
    std::pair<std::string, float> ret = self->handle->add(id_str, datum);
    PyObject *args = PyTuple_New(2);
    PyTuple_SetItem(args, 0, PyUnicode_DecodeUTF8_FromString(ret.first));
    PyTuple_SetItem(args, 1, PyFloat_FromDouble(ret.second));
    return CreateInstanceAndInit(IdWithScoreType, args, NULL);
}

PyObject *AnomalyCalcScore(AnomalyObject *self, PyObject *args)
{
    jubafvconv::datum datum;
    if (!PyDatumToNativeDatum(args, datum))
        return NULL;
    return PyFloat_FromDouble(self->handle->calc_score(datum));
}

PyObject *AnomalyClearRow(AnomalyObject *self, PyObject *args)
{
    std::string id;
    if (!PyUnicodeToUTF8(args, id))
        return NULL;
    self->handle->clear_row(id);
    Py_RETURN_TRUE;
}

PyObject *AnomalyUpdate(AnomalyObject *self, PyObject *args)
{
    std::string id;
    jubafvconv::datum d;
    if (!ParseArgument(args, id, d))
        return NULL;
    return PyFloat_FromDouble(self->handle->update(id, d));
}

PyObject *AnomalyOverwrite(AnomalyObject *self, PyObject *args)
{
    std::string id;
    jubafvconv::datum d;
    if (!ParseArgument(args, id, d))
        return NULL;
    return PyFloat_FromDouble(self->handle->overwrite(id, d));
}

PyObject *AnomalyGetAllRows(AnomalyObject *self, PyObject *args)
{
    return Convert(self->handle->get_all_rows());
}
