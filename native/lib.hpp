#ifndef EMBEDDED_JUBATUS_PYTHON_HPP
#define EMBEDDED_JUBATUS_PYTHON_HPP

#include <Python.h>
#include <jubatus/core/fv_converter/converter_config.hpp>
#include <jubatus/core/fv_converter/datum.hpp>
#include <jubatus/core/driver/anomaly.hpp>
#include <jubatus/core/driver/classifier.hpp>
#include <jubatus/util/text/json.h>

#if PY_MAJOR_VERSION >= 3
#define IS_PY3
#endif

using jubatus::util::lang::shared_ptr;
namespace jubacore = jubatus::core;
namespace jubacomm = jubatus::core::common;
namespace jubadriver = jubatus::core::driver;
namespace jubafvconv = jubatus::core::fv_converter;
namespace jubalang = jubatus::util::lang;
namespace jubajson = jubatus::util::text::json;
namespace jubaframework = jubatus::core::framework;

typedef struct {
    PyObject_HEAD;
    shared_ptr<jubadriver::classifier> handle;
    shared_ptr<std::string> config;
} ClassifierObject;
typedef struct {
    PyObject_HEAD;
    shared_ptr<jubadriver::anomaly> handle;
    shared_ptr<std::string> config;
} AnomalyObject;

int PyUnicodeToUTF8(PyObject *py_str, std::string &out);
int PyBytesToNative(PyObject *py_bin, std::string &out);
int PyNumberToDouble(PyObject *py_num, double &out);
int PyDatumToNativeDatum(PyObject *py_datum, jubafvconv::datum &datum);
int PyDictToJson(PyObject *py_dict, std::string &out);

PyObject* SerializeModel(const std::string& type_, const std::string& config_, const std::string& id_,
                         const msgpack::sbuffer& user_data_buf);
int LoadModelHelper(PyObject *arg, msgpack::unpacked& user_data_buffer,
                    std::string& model_type, std::string& model_id, std::string& model_config,
                    uint64_t *user_data_version, msgpack::object **user_data);

int ClassifierInit(ClassifierObject *self, PyObject *args, PyObject *kwargs);
void ClassifierDealloc(ClassifierObject *self);
PyObject *ClassifierTrain(ClassifierObject *self, PyObject *args);
PyObject *ClassifierClassify(ClassifierObject *self, PyObject *args);
PyObject *ClassifierGetLabels(ClassifierObject *self, PyObject*);
PyObject *ClassifierSetLabel(ClassifierObject *self, PyObject *args);
PyObject *ClassifierDeleteLabel(ClassifierObject *self, PyObject *args);
PyObject *ClassifierDump(ClassifierObject *self, PyObject *args);
PyObject *ClassifierLoad(ClassifierObject *self, PyObject *args);
PyObject *ClassifierClear(ClassifierObject *self, PyObject *args);
PyObject *ClassifierGetConfig(ClassifierObject *self, PyObject *args);

int AnomalyInit(AnomalyObject *self, PyObject *args, PyObject *kwargs);
void AnomalyDealloc(AnomalyObject *self);

static PyMethodDef ClassifierMethods[] = {
    {"train", (PyCFunction)ClassifierTrain, METH_O, ""},
    {"classify", (PyCFunction)ClassifierClassify, METH_O, ""},
    {"get_labels", (PyCFunction)ClassifierGetLabels, METH_NOARGS, ""},
    {"set_label", (PyCFunction)ClassifierSetLabel, METH_O, ""},
    {"delete_label", (PyCFunction)ClassifierDeleteLabel, METH_O, ""},
    {"dump", (PyCFunction)ClassifierDump, METH_NOARGS, ""},
    {"load", (PyCFunction)ClassifierLoad, METH_O, ""},
    {"clear", (PyCFunction)ClassifierClear, METH_NOARGS, ""},
    {"get_config", (PyCFunction)ClassifierGetConfig, METH_NOARGS, ""},
    {NULL}
};

static PyMethodDef AnomalyMethods[] = {
    {NULL}
};

static PyTypeObject ClassifierObjectType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "Classifier",              /* tp_name */
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
static PyTypeObject AnomalyObjectType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "Anomaly",                 /* tp_name */
    sizeof(AnomalyObjectType), /* tp_basicsize */
    0,                         /* tp_itemsize */
    (destructor)AnomalyDealloc, /* tp_dealloc */
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
    "Anomaly",                 /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    AnomalyMethods,            /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)AnomalyInit,     /* tp_init */
    0,                         /* tp_alloc */
    0,                         /* tp_new */
};
static PyTypeObject *_EmbeddedTypes[] = {
    &ClassifierObjectType,
    &AnomalyObjectType,
    NULL
};

#endif
