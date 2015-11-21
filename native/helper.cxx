#include <Python.h>
#include <jubatus/config.hpp>
#include <jubatus/core/common/big_endian.hpp>
#include <jubatus/core/framework/packer.hpp>
#include <jubatus/core/framework/stream_writer.hpp>
#include <jubatus/server/common/crc32.hpp>
#include <cstdio>
#include <cstring>
#include <ctime>
#include "lib.hpp"

using jubatus::core::common::write_big_endian;
using jubatus::core::common::read_big_endian;

using jubatus::server::common::calc_crc32;

int PyLongToNative(PyObject *py_long, long &out)
{
#ifdef IS_PY3
    if (!PyLong_Check(py_long))
        return 0;
    out = PyLong_AsLong(py_long);
#else
    if (!PyLong_Check(py_long) && !PyInt_Check(py_long))
        return 0;
    out = (PyInt_Check(py_long) ? PyInt_AsLong(py_long) : PyLong_AsLong(py_long));
#endif
    return 1;
}

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
int PyDatumToNativeDatum(PyObject *py_datum, jubafvconv::datum &datum)
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
        PyObject *true_obj = Py_True;
        PyObject *false_obj = Py_False;
        Py_INCREF(true_obj);
        Py_INCREF(false_obj);
        PyDict_SetItemString(kwargs, "ensure_ascii", false_obj); // steals false_obj ref
        PyDict_SetItemString(kwargs, "sort_keys", true_obj); // steals true_obj ref
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

PyObject* NativeDatumToPyDatum(const jubafvconv::datum &datum)
{
#ifdef IS_PY3
    static PyTypeObject *DatumType = NULL;
#else
    static PyClassObject *DatumType = NULL;
#endif
    if (!DatumType) {
        PyObject *m = PyImport_ImportModule("jubatus.common");
        if (m) {
#ifdef IS_PY3
            DatumType = (PyTypeObject*)PyObject_GetAttrString(m, "Datum");
#else
            DatumType = (PyClassObject*)PyObject_GetAttrString(m, "Datum");
#endif
            Py_DECREF(m);
        }
    }

    PyObject *dict = PyDict_New();
    for (jubafvconv::datum::sv_t::const_iterator it = datum.string_values_.begin();
         it != datum.string_values_.end(); it ++) {
        PyObject *key = PyUnicode_DecodeUTF8(it->first.data(),
                                             it->first.size(), NULL);
        PyObject *val = PyUnicode_DecodeUTF8(it->second.data(),
                                             it->second.size(), NULL);
        PyDict_SetItem(dict, key, val);
    }
    for (jubafvconv::datum::nv_t::const_iterator it = datum.num_values_.begin();
         it != datum.num_values_.end(); it ++) {
        PyObject *key = PyUnicode_DecodeUTF8(it->first.data(),
                                             it->first.size(), NULL);
        PyObject *val = PyFloat_FromDouble(it->second);
        PyDict_SetItem(dict, key, val);
    }
    for (jubafvconv::datum::sv_t::const_iterator it = datum.binary_values_.begin();
         it != datum.binary_values_.end(); it ++) {
        PyObject *key = PyUnicode_DecodeUTF8(it->first.data(),
                                             it->first.size(), NULL);
#ifdef IS_PY3
        PyObject *val = PyBytes_FromStringAndSize(it->second.data(),
                                                  it->second.size());
#else
        PyObject *val = PyString_FromStringAndSize(it->second.data(),
                                                   it->second.size());
#endif
        PyDict_SetItem(dict, key, val);
    }

    PyObject *args = PyTuple_New(1);
    PyTuple_SetItem(args, 0, dict);
#ifdef IS_PY3
    PyObject *py_datum = DatumType->tp_new(DatumType, args, NULL);
    DatumType->tp_init(py_datum, args, NULL);
#else
    PyObject *py_datum = PyInstance_New((PyObject*)DatumType, args, NULL);
#endif
    return py_datum;
}

static const char magic_number[8] = "jubatus";
static const uint64_t system_data_container_version = 1;
static const uint64_t format_version = 1;

static uint32_t jubatus_version_major = -1;
static uint32_t jubatus_version_minor = -1;
static uint32_t jubatus_version_maintenance = -1;

static void _InitJubatusVersion()
{
    if (jubatus_version_major == static_cast<uint32_t>(-1)) {
        int major, minor, maintenance;
        std::sscanf(JUBATUS_VERSION, "%d.%d.%d", &major, &minor, &maintenance);  // NOLINT
        jubatus_version_major = major;
        jubatus_version_minor = minor;
        jubatus_version_maintenance = maintenance;
    }
}

PyObject* SerializeModel(const std::string& type_, const std::string& config_, const std::string& id_,
                         const msgpack::sbuffer& user_data_buf)
{
    // ダンプイメージはコア実装ではなくサーバ実装に依存するため
    // Jubatusサーバのダンプと互換性のある方法で出力する
    // Ref: jubatus/server/framework/save_load.cpp
    _InitJubatusVersion();

    msgpack::sbuffer system_data_buf;
    {
        jubaframework::stream_writer<msgpack::sbuffer> st(system_data_buf);
        jubaframework::jubatus_packer jp(st);
        jubaframework::packer packer(jp);
        packer.pack_array(5);
        packer.pack_uint64(system_data_container_version);
        packer.pack_uint64(std::time(NULL));
        packer.pack(type_);
        packer.pack(id_);
        packer.pack(config_);
    }

    char header_buf[48];
    std::memcpy(header_buf, magic_number, 8);
    write_big_endian(format_version, &header_buf[8]);
    write_big_endian(jubatus_version_major, &header_buf[16]);
    write_big_endian(jubatus_version_minor, &header_buf[20]);
    write_big_endian(jubatus_version_maintenance, &header_buf[24]);
    // write_big_endian(crc32, &header_buf[28]);  // skipped
    write_big_endian(static_cast<uint64_t>(system_data_buf.size()),
                     &header_buf[32]);
    write_big_endian(static_cast<uint64_t>(user_data_buf.size()),
                     &header_buf[40]);

    uint32_t crc32 = calc_crc32(header_buf, 28);
    crc32 = calc_crc32(&header_buf[32], 16, crc32);
    crc32 = calc_crc32(system_data_buf.data(), system_data_buf.size(), crc32);
    crc32 = calc_crc32(user_data_buf.data(), user_data_buf.size(), crc32);
    write_big_endian(crc32, &header_buf[28]);

    size_t ret_size = sizeof(header_buf) + system_data_buf.size() + user_data_buf.size();
    char *temp = new char[ret_size];
    char *p = temp;
    std::memcpy(p, header_buf, sizeof(header_buf));
    p += sizeof(header_buf);
    std::memcpy(p, system_data_buf.data(), system_data_buf.size());
    p += system_data_buf.size();
    std::memcpy(p, user_data_buf.data(), user_data_buf.size());
    p += user_data_buf.size();
#ifdef IS_PY3
    PyObject *ret = PyBytes_FromStringAndSize(temp, ret_size);
#else
    PyObject *ret = PyString_FromStringAndSize(temp, ret_size);
#endif
    delete[] temp;
    return ret;
}

int LoadModelHelper(PyObject *arg, msgpack::unpacked& user_data_buffer,
                    std::string& model_type, std::string& model_id, std::string& model_config,
                    uint64_t *user_data_version, msgpack::object **user_data)
{
    Py_buffer view;
    PyObject *bytes_obj = NULL;
    int ret = 0;

    if (PyObject_GetBuffer(arg, &view, PyBUF_SIMPLE) != 0) {
        PyObject *read_attr = PyObject_GetAttrString(arg, "read");
        if (!read_attr)
            return 0;
        bytes_obj = PyObject_CallObject(read_attr, NULL);
        Py_DECREF(read_attr);
        if (PyObject_GetBuffer(bytes_obj, &view, PyBUF_SIMPLE) != 0) {
            Py_DECREF(bytes_obj);
            return 0;
        }
    }

    _InitJubatusVersion();
    do {
        char *p = (char*)view.buf;
        char *sys = &p[48];
        if (std::memcmp(p, magic_number, 8) != 0)
            break;
        if (read_big_endian<uint64_t>(&p[8]) != format_version)
            break;
        uint32_t jubatus_major_read = read_big_endian<uint32_t>(&p[16]);
        uint32_t jubatus_minor_read = read_big_endian<uint32_t>(&p[20]);
        uint32_t jubatus_maintenance_read = read_big_endian<uint32_t>(&p[24]);
        if (jubatus_major_read != jubatus_version_major
            || jubatus_minor_read != jubatus_version_minor
            || jubatus_maintenance_read != jubatus_version_maintenance)
            break;
        uint32_t crc32_expected = read_big_endian<uint32_t>(&p[28]);
        uint64_t system_data_size = read_big_endian<uint64_t>(&p[32]);
        uint64_t user_data_size = read_big_endian<uint64_t>(&p[40]);
        if (48 + system_data_size + user_data_size != view.len)
            break;
        char *user = &sys[system_data_size];
        uint32_t crc32_actual = calc_crc32(p, 28);
        crc32_actual = calc_crc32(&p[32], 16, crc32_actual);
        crc32_actual = calc_crc32(sys, system_data_size, crc32_actual);
        crc32_actual = calc_crc32(user, user_data_size, crc32_actual);
        if (crc32_actual != crc32_expected)
            break;

        {
            msgpack::unpacked unpacked;
            msgpack::unpack(&unpacked, sys, system_data_size);
            if (unpacked.get().type != msgpack::type::ARRAY)
                break;
            msgpack::object_array& sc = unpacked.get().via.array;
            if (sc.size != 5 || sc.ptr[0].via.u64 != system_data_container_version
                || sc.ptr[2].type != msgpack::type::RAW
                || sc.ptr[3].type != msgpack::type::RAW
                || sc.ptr[4].type != msgpack::type::RAW)
                break;
            model_type.assign(sc.ptr[2].via.raw.ptr, sc.ptr[2].via.raw.size);
            model_id.assign(sc.ptr[3].via.raw.ptr, sc.ptr[3].via.raw.size);
            model_config.assign(sc.ptr[4].via.raw.ptr, sc.ptr[4].via.raw.size);
        }

        {
            msgpack::unpack(&user_data_buffer, user, user_data_size);
            if (user_data_buffer.get().type != msgpack::type::ARRAY)
                break;
            msgpack::object_array& sc = user_data_buffer.get().via.array;
            if (sc.size != 2 || sc.ptr[0].type != msgpack::type::POSITIVE_INTEGER)
                break;
            *user_data_version = sc.ptr[0].via.u64;
            *user_data = &sc.ptr[1];
        }

        ret = 1;
    } while (0);

    if (bytes_obj)
        Py_DECREF(bytes_obj);
    return ret;
}
