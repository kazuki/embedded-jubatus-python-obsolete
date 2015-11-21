#ifndef EMBEDDED_JUBATUS_PYTHON_HELPER_HPP
#define EMBEDDED_JUBATUS_PYTHON_HELPER_HPP

#include "lib.hpp"

int PyLongToNative(PyObject *py_long, long &out);
int PyUnicodeToUTF8(PyObject *py_str, std::string &out);
int PyBytesToNative(PyObject *py_bin, std::string &out);
int PyNumberToDouble(PyObject *py_num, double &out);
int PyDatumToNativeDatum(PyObject *py_datum, jubafvconv::datum &datum);
int PyDictToJson(PyObject *py_dict, std::string &out);
PyObject* NativeDatumToPyDatum(const jubafvconv::datum &datum);

PyObject* SerializeModel(const std::string& type_, const std::string& config_, const std::string& id_,
                         const msgpack::sbuffer& user_data_buf);
int LoadModelHelper(PyObject *arg, msgpack::unpacked& user_data_buffer,
                    std::string& model_type, std::string& model_id, std::string& model_config,
                    uint64_t *user_data_version, msgpack::object **user_data);

int ParseArgument(PyObject *args, std::string &out0, long &out1);
int ParseArgument(PyObject *args, jubafvconv::datum &out0, long &out1);
int ParseArgument(PyObject *args, std::string &out0, jubafvconv::datum &out1);

PyObject *Convert(const std::vector<std::string> &list);
PyObject *ConvertToIdWithScoreList(const std::vector<std::pair<std::string, float> > &list, PyTypeObject *type);

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
int ParseInitArgsWithoutConv(T* self, PyObject *args, std::string &out_method,
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
        out_config = jubacomm::jsonconfig::config(config_json["parameter"]);
        self->config.reset(new std::string(str_config_json));
    } catch (std::exception &e) {
        PyErr_SetString(PyExc_TypeError, e.what());
        return 0;
    }
    return 1;
}

#endif
