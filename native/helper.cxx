#include <Python.h>
#include "lib.hpp"

/*
 * Pythonの文字列型(Unicode, Py2のみString)を，UTF8で符号化されたstd::stringに変換する
 */
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

int PyBytesToNative(PyObject *py_bin, std::string &out)
{
#ifdef IS_PY3
    char *p = PyBytes_AsString(py_bin);
    if (!p) return 0;
    out.assign(p, PyBytes_Size(py_bin));
#else
    char *p = PyString_AsString(py_bin);
    if (!p) return 0;
    out.assign(p, PyString_Size(py_bin));
#endif
    return 1;
}

// PyObject(int/float/long) => double
int PyNumberToDouble(PyObject *py_num, double &out)
{
    if (PyFloat_Check(py_num)) {
        out = PyFloat_AsDouble(py_num);
    } else if (PyLong_Check(py_num)) {
        out = (double)PyLong_AsLong(py_num);
#ifndef IS_PY3
    } else if (PyInt_Check(py_num)) {
        out = (double)PyInt_AsLong(py_num);
#endif
    } else {
        return 0;
    }
    return 1;
}

// jubatus.common.Datum(Python) => jubatus::core::fv_converter::datum(C++)
int PyDatumToNativeDatum(PyObject *py_datum, jubatus::core::fv_converter::datum &datum)
{
    static const char *FIELD_NAMES[] = {
        "string_values", "num_values", "binary_values"
    };

    std::string key, str_value;
    double num_value;
    datum.string_values_.clear();
    datum.num_values_.clear();
    datum.binary_values_.clear();

    for (int field_idx = 0; field_idx < 3; ++field_idx) {
        PyObject *py_values = PyObject_GetAttrString(py_datum, FIELD_NAMES[field_idx]);
        if (!py_values) continue;

        for (Py_ssize_t i = 0; i < PyList_Size(py_values); ++i) {
            PyObject *item = PyList_GetItem(py_values, i);
            PyObject *name = PyList_GetItem(item, 0);
            if (!name) return 0;
            if (!PyUnicodeToUTF8(name, key)) return 0;

            PyObject *value = PyList_GetItem(item, 1);
            if (!value) return 0;
            if (field_idx == 0) {
                if (!PyUnicodeToUTF8(value, str_value)) return 0;
                datum.string_values_.push_back(std::pair<std::string, std::string>(key, str_value));
            } else if (field_idx == 1) {
                if (!PyNumberToDouble(value, num_value)) return 0;
                datum.num_values_.push_back(std::pair<std::string, double>(key, num_value));
            } else {
                // 一応変換のコードを書くが，JubatusのPythonクライアントのDatumはbinaryに対応していないため
                // ここが呼び出されることは無い
                if (!PyBytesToNative(value, str_value)) return 0;
                datum.binary_values_.push_back(std::pair<std::string, std::string>(key, str_value));
            }
        }
    }
    return 1;
}

int PyDictToJson(PyObject *py_dict, std::string &out)
{
    PyObject *m = PyImport_ImportModule("json"); // return: new-ref
    if (!m) return 0;
    int ret_code = 0;
    PyObject *dump_method = PyObject_GetAttrString(m, "dumps"); // return: new-ref
    if (dump_method) {
        PyObject *args = PyTuple_New(1); // return: new-ref
        Py_INCREF(py_dict);
        PyTuple_SetItem(args, 0, py_dict); // steals py_dict ref
        PyObject *kwargs = PyDict_New(); // return: new-ref
        PyObject *false_obj = Py_False;
        Py_INCREF(Py_False);
        PyDict_SetItemString(kwargs, "ensure_ascii", false_obj); // steals false_obj ref
        PyObject *json_str = PyObject_Call(dump_method, args, kwargs); // return: new-ref
        if (json_str) {
            if (PyUnicodeToUTF8(json_str, out))
                ret_code = 1;
            Py_DECREF(json_str);
        }
        Py_DECREF(args);
        Py_DECREF(kwargs);
        Py_DECREF(dump_method);
    }
    Py_DECREF(m);
    return ret_code;
}
