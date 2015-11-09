#ifndef EMBEDDED_JUBATUS_PYTHON_HPP
#define EMBEDDED_JUBATUS_PYTHON_HPP

#include <Python.h>
#include <jubatus/core/fv_converter/converter_config.hpp>
#include <jubatus/core/fv_converter/datum.hpp>
#include <jubatus/core/classifier/classifier_base.hpp>

#if PY_MAJOR_VERSION >= 3
#define IS_PY3
#endif

typedef struct {
    PyObject_HEAD;
    jubatus::util::lang::shared_ptr<jubatus::core::classifier::classifier_base> handle;
    jubatus::util::lang::shared_ptr<jubatus::core::fv_converter::datum_to_fv_converter> converter;
} ClassifierObject;

int PyUnicodeToUTF8(PyObject *py_str, std::string &out);
int PyDatumToNativeDatum(PyObject *py_datum, jubatus::core::fv_converter::datum &datum);

int ClassifierInit(ClassifierObject *self, PyObject *args, PyObject *kwargs);
void ClassifierDealloc(ClassifierObject *self);
PyObject *ClassifierTrain(ClassifierObject *self, PyObject *args);
PyObject *ClassifierClassify(ClassifierObject *self, PyObject *args);

static PyMethodDef ClassifierMethods[] = {
    {"train", (PyCFunction)ClassifierTrain, METH_VARARGS, ""},
    {"classify", (PyCFunction)ClassifierClassify, METH_VARARGS, ""},
    {NULL}
};

static PyTypeObject ClassifierObjectType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "native.Classifier",       /* tp_name */
    sizeof(ClassifierObjectType), /* tp_basicsize */
    0,                         /* tp_itemsize */
    (destructor)ClassifierDealloc, /* tp_dealloc */
    0,                         /* tp_print */
    0,                         /* tp_getattr */
    0,                         /* tp_setattr */
    0,                         /* tp_reserved */
    0,                         /* tp_repr */
    0,                         /* tp_as_number */
    0,                         /* tp_as_sequence */
    0,                         /* tp_as_mapping */
    0,                         /* tp_hash  */
    0,                         /* tp_call */
    0,                         /* tp_str */
    0,                         /* tp_getattro */
    0,                         /* tp_setattro */
    0,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, /* tp_flags */
    "Classifier",              /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    ClassifierMethods,         /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)ClassifierInit,  /* tp_init */
    0,                         /* tp_alloc */
    0,                         /* tp_new */
};
static PyTypeObject *_EmbeddedTypes[] = {
    &ClassifierObjectType,
    NULL
};

#endif
