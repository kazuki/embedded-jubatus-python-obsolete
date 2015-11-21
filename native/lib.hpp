#ifndef EMBEDDED_JUBATUS_PYTHON_HPP
#define EMBEDDED_JUBATUS_PYTHON_HPP

#include <Python.h>
#include <jubatus/core/common/jsonconfig.hpp>
#include <jubatus/core/fv_converter/converter_config.hpp>
#include <jubatus/core/fv_converter/datum.hpp>
#include <jubatus/core/driver/anomaly.hpp>
#include <jubatus/core/driver/classifier.hpp>
#include <jubatus/core/driver/recommender.hpp>
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
    uint64_t idgen;
} AnomalyObject;
typedef struct {
    PyObject_HEAD;
    shared_ptr<jubadriver::recommender> handle;
    shared_ptr<std::string> config;
} RecommenderObject;

int PyLongToNative(PyObject *py_long, long &out);
int PyUnicodeToUTF8(PyObject *py_str, std::string &out);
int PyBytesToNative(PyObject *py_bin, std::string &out);
int PyNumberToDouble(PyObject *py_num, double &out);
int PyDatumToNativeDatum(PyObject *py_datum, jubafvconv::datum &datum);
int PyDictToJson(PyObject *py_dict, std::string &out);
PyObject* NativeDatumToPyDatum(const jubafvconv::datum &datum);

inline int LookupTypeObject(const char *module_name, const char *type_name, PyTypeObject **type) {
    if (*type) return 1;
    PyObject *m = PyImport_ImportModule(module_name);
    if (!m) return 0;
    *type = (PyTypeObject*)PyObject_GetAttrString(m, type_name);
    Py_DECREF(m);
    return (*type ? 1 : 0);
}
inline PyObject* CreateInstanceAndInit(PyTypeObject *type, PyObject *args, PyObject **kwargs) {
#ifdef IS_PY3
    PyObject *obj = type->tp_new(type, args, NULL);
    type->tp_init(obj, args, NULL);
    return obj;
#else
    return PyInstance_New((PyObject*)type, args, NULL);
#endif
}
inline PyObject *PyUnicode_DecodeUTF8_FromString(const std::string &str) {
    return PyUnicode_DecodeUTF8(str.data(), str.size(), NULL);
}
PyObject* SerializeModel(const std::string& type_, const std::string& config_, const std::string& id_,
                         const msgpack::sbuffer& user_data_buf);
int LoadModelHelper(PyObject *arg, msgpack::unpacked& user_data_buffer,
                    std::string& model_type, std::string& model_id, std::string& model_config,
                    uint64_t *user_data_version, msgpack::object **user_data);

template<typename T>
int ParseInitArgs(T* self, PyObject *args, std::string &out_method,
                  jubafvconv::converter_config &out_fvconv_config,
                  jubacomm::jsonconfig::config &out_config) {
    PyObject *py_config_obj;
    if (!PyArg_ParseTuple(args, "O!", &PyDict_Type, &py_config_obj))
        return 0;
    std::string str_config_json;
    if (!PyDictToJson(py_config_obj, str_config_json))
        return 0;

    jubajson::json config_json = jubalang::lexical_cast<jubajson::json>(str_config_json);
    jubajson::json_string *method_value = (jubajson::json_string*)config_json["method"].get();
    if (!method_value || method_value->type() != jubajson::json::String) {
        PyErr_SetString(PyExc_TypeError, "invalid config");
        return 0;
    }
    out_method.assign(method_value->get());

    try {
        jubajson::from_json(config_json["converter"], out_fvconv_config);
        out_config = jubacomm::jsonconfig::config(config_json["parameter"]);
        self->config.reset(new std::string(str_config_json));
    } catch (std::exception &e) {
        PyErr_SetString(PyExc_TypeError, e.what());
        return 0;
    }
    return 1;
}

template<typename T>
PyObject *CommonApiClear(T *self, PyObject *args) {
    self->handle->clear();
    Py_RETURN_NONE;
}
template<typename T>
PyObject *CommonApiGetConfig(T *self, PyObject *args) {
    return PyUnicode_DecodeUTF8_FromString(*(self->config));
}

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
PyObject *AnomalyAdd(AnomalyObject *self, PyObject *args);
PyObject *AnomalyCalcScore(AnomalyObject *self, PyObject *args);

int RecommenderInit(RecommenderObject *self, PyObject *args, PyObject *kwargs);
void RecommenderDealloc(RecommenderObject *self);
PyObject *RecommenderClearRow(RecommenderObject *self, PyObject *args);
PyObject *RecommenderUpdateRow(RecommenderObject *self, PyObject *args);
PyObject *RecommenderCompleteRowFromId(RecommenderObject *self, PyObject *args);
PyObject *RecommenderCompleteRowFromDatum(RecommenderObject *self, PyObject *args);
PyObject *RecommenderSimilarRowFromId(RecommenderObject *self, PyObject *args);
PyObject *RecommenderSimilarRowFromDatum(RecommenderObject *self, PyObject *args);
PyObject *RecommenderDecodeRow(RecommenderObject *self, PyObject *args);
PyObject *RecommenderGetAllRows(RecommenderObject *self, PyObject *args);
PyObject *RecommenderCalcSimilarity(RecommenderObject *self, PyObject *args);
PyObject *RecommenderCalcL2Norm(RecommenderObject *self, PyObject *args);

static PyMethodDef ClassifierMethods[] = {
    {"train", (PyCFunction)ClassifierTrain, METH_O, ""},
    {"classify", (PyCFunction)ClassifierClassify, METH_O, ""},
    {"get_labels", (PyCFunction)ClassifierGetLabels, METH_NOARGS, ""},
    {"set_label", (PyCFunction)ClassifierSetLabel, METH_O, ""},
    {"delete_label", (PyCFunction)ClassifierDeleteLabel, METH_O, ""},
    {"dump", (PyCFunction)ClassifierDump, METH_NOARGS, ""},
    {"load", (PyCFunction)ClassifierLoad, METH_O, ""},
    {"clear", (PyCFunction)CommonApiClear<ClassifierObject>, METH_NOARGS, ""},
    {"get_config", (PyCFunction)CommonApiGetConfig<ClassifierObject>, METH_NOARGS, ""},
    {NULL}
};

static PyMethodDef AnomalyMethods[] = {
    {"add", (PyCFunction)AnomalyAdd, METH_O, ""},
    {"calc_score", (PyCFunction)AnomalyCalcScore, METH_O, ""},
    {"clear", (PyCFunction)CommonApiClear<AnomalyObject>, METH_NOARGS, ""},
    {"get_config", (PyCFunction)CommonApiGetConfig<AnomalyObject>, METH_NOARGS, ""},
    {NULL}
};

static PyMethodDef RecommenderMethods[] = {
    {"clear_row", (PyCFunction)RecommenderClearRow, METH_O, ""},
    {"update_row", (PyCFunction)RecommenderUpdateRow, METH_VARARGS, ""},
    {"complete_row_from_id", (PyCFunction)RecommenderCompleteRowFromId, METH_O, ""},
    {"complete_row_from_datum", (PyCFunction)RecommenderCompleteRowFromDatum, METH_O, ""},
    {"similar_row_from_id", (PyCFunction)RecommenderSimilarRowFromId, METH_VARARGS, ""},
    {"similar_row_from_datum", (PyCFunction)RecommenderSimilarRowFromDatum, METH_VARARGS, ""},
    {"decode_row", (PyCFunction)RecommenderDecodeRow, METH_O, ""},
    {"get_all_rows", (PyCFunction)RecommenderGetAllRows, METH_NOARGS, ""},
    {"calc_similarity", (PyCFunction)RecommenderCalcSimilarity, METH_VARARGS, ""},
    {"calc_l2norm", (PyCFunction)RecommenderCalcL2Norm, METH_O, ""},
    {"clear", (PyCFunction)CommonApiClear<RecommenderObject>, METH_NOARGS, ""},
    {"get_config", (PyCFunction)CommonApiGetConfig<RecommenderObject>, METH_NOARGS, ""},
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
static PyTypeObject RecommenderObjectType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "Recommender",                 /* tp_name */
    sizeof(RecommenderObjectType), /* tp_basicsize */
    0,                         /* tp_itemsize */
    (destructor)RecommenderDealloc, /* tp_dealloc */
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
    "Recommender",             /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    RecommenderMethods,        /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)RecommenderInit, /* tp_init */
    0,                         /* tp_alloc */
    0,                         /* tp_new */
};
static PyTypeObject *_EmbeddedTypes[] = {
    &ClassifierObjectType,
    &AnomalyObjectType,
    &RecommenderObjectType,
    NULL
};

#endif
