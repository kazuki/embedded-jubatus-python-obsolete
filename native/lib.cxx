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
