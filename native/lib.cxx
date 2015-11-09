#include <Python.h>
#include <jubatus/core_config.hpp>
#include <cstring>
#include "lib.hpp"

static PyObject *get_core_version(PyObject *self, PyObject *args)
{
    return PyUnicode_DecodeUTF8(JUBATUS_CORE_VERSION,
                                std::strlen(JUBATUS_CORE_VERSION), NULL);
}

static PyMethodDef JubatusEmbeddedMethods[] = {
    {"get_core_version", get_core_version, METH_VARARGS, "Get jubatus_core module version"},
    {NULL, NULL, 0, NULL}
};

#ifdef IS_PY3
static struct PyModuleDef JubatusEmbeddedModule = {
    PyModuleDef_HEAD_INIT,
    "embedded",
    NULL,
    -1,
    JubatusEmbeddedMethods
};
#define MODINIT_RETURN_WRAP(x) x
#else
#define MODINIT_RETURN_WRAP(x)
#endif

extern "C" {
PyMODINIT_FUNC
#ifdef IS_PY3
PyInit_embedded(void)
#else
initembedded(void)
#endif
{
    for (int i = 0; _EmbeddedTypes[i]; ++i) {
        _EmbeddedTypes[i]->tp_new = PyType_GenericNew;
        if (PyType_Ready(_EmbeddedTypes[i]) < 0)
            return MODINIT_RETURN_WRAP(NULL);
    }

    PyObject *m =
#ifdef IS_PY3
        PyModule_Create(&JubatusEmbeddedModule);
#else
        Py_InitModule("embedded", JubatusEmbeddedMethods);
#endif
    if (!m)
        return MODINIT_RETURN_WRAP(NULL);

    for (int i = 0; _EmbeddedTypes[i]; ++i) {
        Py_INCREF(_EmbeddedTypes[i]);
        PyModule_AddObject(m,
                           _EmbeddedTypes[i]->tp_name + 7,
                           (PyObject*)_EmbeddedTypes[i]);
    }
    return MODINIT_RETURN_WRAP(m);
}
}

int PyUnicodeToUTF8(PyObject *py_str, std::string &out)
{
#ifdef IS_PY3
    PyObject *utf8 = PyUnicode_AsUTF8String(py_str);
    if (!utf8) return 0;
    out.assign(PyBytes_AsString(utf8), PyBytes_Size(utf8));
    Py_DECREF(utf8);
#else
    if (PyString_Check(py_str)) {
        out.assign(PyString_AsString(py_str), PyString_Size(py_str));
    } else {
        PyObject *utf8 = PyUnicode_AsUTF8String(py_str);
        if (!utf8) return 0;
        out.assign(PyString_AsString(utf8), PyString_Size(utf8));
        Py_DECREF(utf8);
    }
#endif
    return 1;
}

int PyDatumToNativeDatum(PyObject *py_datum, jubatus::core::fv_converter::datum &datum)
{
    std::string str;
    datum.string_values_.clear();
    datum.num_values_.clear();
    datum.binary_values_.clear();

    PyObject *py_strs = PyObject_GetAttrString(py_datum, "string_values");
    if (py_strs) {
    }
    PyObject *py_nums = PyObject_GetAttrString(py_datum, "num_values");
    if (py_nums) {
        for (Py_ssize_t j = 0; j < PyList_Size(py_nums); ++j) {
            PyObject *item = PyList_GetItem(py_nums, j);
            PyObject *name = PyList_GetItem(item, 0);
            if (!name) return 0;
            if (!PyUnicodeToUTF8(name, str)) return 0;

            PyObject *value = PyList_GetItem(item, 1);
            if (!value) return 0;
            double raw_value = 0.0;
            if (PyFloat_Check(value)) {
                raw_value = PyFloat_AsDouble(value);
            } else if (PyLong_Check(value)) {
                raw_value = (double)PyLong_AsLong(value);
#ifndef IS_PY3
            } else if (PyInt_Check(value)) {
                raw_value = (double)PyInt_AsLong(value);
#endif
            } else {
                return 0;
            }
            datum.num_values_.push_back(std::pair<std::string, double>(str, raw_value));
        }
    }
    PyObject *py_bins = PyObject_GetAttrString(py_datum, "binary_values");
    if (py_bins) {
    }

    return 1;
}
