#include <jubatus/core/storage/storage_factory.hpp>
#include <jubatus/core/classifier/classifier_factory.hpp>
#include <jubatus/util/lang/cast.h>
#include "lib.hpp"
#include "helper.hpp"

static PyTypeObject *EstimateResultType = NULL;
static const std::string TYPE("classifier");
static const uint64_t USER_DATA_VERSION = 1;

int ClassifierInit(ClassifierObject *self, PyObject *args, PyObject *kwargs)
{
    std::string method;
    jubafvconv::converter_config fvconv_config;
    jubacomm::jsonconfig::config config;
    if (!ParseInitArgs(self, args, method, fvconv_config, config))
        return -1;
    try {
        self->handle.reset(
            new jubadriver::classifier(
                jubacore::classifier::classifier_factory::create_classifier(
                    method, config,
                    jubacore::storage::storage_factory::create_storage("local")),
                jubafvconv::make_fv_converter(fvconv_config, NULL)));
    } catch (std::exception &e) {
        PyErr_SetString(PyExc_TypeError, e.what());
        return -1;
    }
    if (!LookupTypeObject("jubatus.classifier.types", "EstimateResult", &EstimateResultType))
        return -1;
    return 0;
}

void ClassifierDealloc(ClassifierObject *self)
{
    self->handle.reset();
    self->config.reset();
    Py_TYPE(self)->tp_free((PyObject*)self);
}

PyObject *ClassifierTrain(ClassifierObject *self, PyObject *list)
{
    std::string str;
    jubafvconv::datum d;
    if (!PyList_Check(list))
        return NULL;
    for (Py_ssize_t i = 0; i < PyList_Size(list); ++i) {
        PyObject *labeled_datum = PyList_GetItem(list, i); // borrowed
        if (!labeled_datum)
            return NULL;
        PyObject *label = PyTuple_GetItem(labeled_datum, 0); // borrowed
        if (!label)
            return NULL;
        PyObject *datum = PyTuple_GetItem(labeled_datum, 1); // borrowed
        if (!datum)
            return NULL;
        if (!PyUnicodeToUTF8(label, str))
            return NULL;

        PyDatumToNativeDatum(datum, d);
        CATCH_CPP_EXCEPTION_AND_RETURN_NULL(self->handle->train(str, d));
    }
    Py_RETURN_NONE;
}

PyObject *ClassifierClassify(ClassifierObject *self, PyObject *list)
{
    jubafvconv::datum d;
    if (!PyList_Check(list))
        return NULL;
    PyObject *out = PyList_New(PyList_Size(list));
    for (Py_ssize_t i = 0; i < PyList_Size(list); ++i) {
        PyObject *datum = PyList_GetItem(list, i);
        if (!PyDatumToNativeDatum(datum, d)) {
            Py_DECREF(out);
            return NULL;
        }
        jubacore::classifier::classify_result ret;
        CATCH_CPP_EXCEPTION_AND_RETURN_NULL2(ret = self->handle->classify(d),
                                             Py_DECREF(out));
        PyObject *tmp = PyList_New(ret.size());
        for (int j = 0; j < ret.size(); ++j) {
            PyObject *args = PyTuple_New(2);
            PyTuple_SetItem(args, 0, PyUnicode_DecodeUTF8_FromString(ret[j].label));
            PyTuple_SetItem(args, 1, PyFloat_FromDouble(ret[j].score));
            PyList_SetItem(tmp, j, CreateInstanceAndInit(EstimateResultType, args, NULL));
        }
        PyList_SetItem(out, i, tmp);
    }
    return out;
}

PyObject *ClassifierGetLabels(ClassifierObject *self, PyObject*)
{
    jubatus::core::classifier::labels_t labels;
    CATCH_CPP_EXCEPTION_AND_RETURN_NULL(labels = self->handle->get_labels());

    PyObject *ret = PyDict_New();
    for (jubatus::core::classifier::labels_t::iterator it = labels.begin(); it != labels.end(); ++it) {
        PyObject *key = PyUnicode_DecodeUTF8_FromString(it->first);
        PyObject *val = PyLong_FromLong(it->second);
        PyDict_SetItem(ret, key, val);
        Py_DECREF(key);
        Py_DECREF(val);
    }
    return ret;
}

PyObject *ClassifierSetLabel(ClassifierObject *self, PyObject *args)
{
    std::string label;
    if (!PyUnicodeToUTF8(args, label))
        return NULL;
    CATCH_CPP_EXCEPTION_AND_RETURN_NULL(self->handle->set_label(label));
    Py_RETURN_NONE;
}

PyObject *ClassifierDeleteLabel(ClassifierObject *self, PyObject *args)
{
    std::string label;
    if (!PyUnicodeToUTF8(args, label))
        return NULL;
    CATCH_CPP_EXCEPTION_AND_RETURN_NULL(self->handle->delete_label(label));
    Py_RETURN_NONE;
}

PyObject *ClassifierDump(ClassifierObject *self, PyObject *args)
{
    return CommonApiDump(self, TYPE, USER_DATA_VERSION);
}

PyObject *ClassifierLoad(ClassifierObject *self, PyObject *args)
{
    return CommonApiLoad(self, args, TYPE, USER_DATA_VERSION);
}

static PyTypeObject* GetNDArrayType(){
    static bool first = true;
    static PyTypeObject *type = NULL;
    if (first) {
        first = false;
        if (!LookupTypeObject("numpy", "ndarray", &type)) {
            type = NULL;
        }
    }
    return type;
}

static PyTypeObject* GetCSRMatrixType(){
    static bool first = true;
    static PyTypeObject *type = NULL;
    if (first) {
        first = false;
        if (!LookupTypeObject("scipy.sparse", "csr_matrix", &type)) {
            type = NULL;
        }
    }
    return type;
}

struct NumpyArrayObject {
  PyObject_HEAD
  char *data;
  int nd;
  long *dimensions;
  long *strides;
  PyObject *base;
};

enum NumPyDataType {
  DTYPE_I4,
  DTYPE_I8,
  DTYPE_F4,
  DTYPE_F8,
};
static NumPyDataType GetNumPyDataType(PyObject *obj)
{
    static const std::string f4_little ("<f4");
    static const std::string f8_little ("<f8");
    static const std::string i4_little ("<i4");
    static const std::string i8_little ("<i8");

    ScopedPyRef py_dtype(PyObject_GetAttrString(obj, "dtype"));
    ScopedPyRef py_dtype_str(PyObject_GetAttrString(py_dtype.get(), "str"));
    std::string dtype_str;
    PyUnicodeToUTF8(py_dtype_str.get(), dtype_str);
    if (dtype_str == f8_little) {
      return DTYPE_F8;
    } else if (dtype_str == f4_little) {
      return DTYPE_F4;
    } else if (dtype_str == i8_little) {
      return DTYPE_I8;
    } else if (dtype_str == i4_little) {
      return DTYPE_I4;
    } else {
      throw std::runtime_error("not supported dtype");
    }
}

static void NDArrayToDatum(PyObject *py_X, std::vector<jubafvconv::datum>& out)
{
  NumPyDataType dtype = GetNumPyDataType(py_X);
  const NumpyArrayObject *emu_obj = reinterpret_cast<const NumpyArrayObject*>(py_X);
  if (emu_obj->nd != 2)
    throw std::runtime_error("invalid dimensions");
  if (emu_obj->base)
    throw std::runtime_error("not supported cow ndarray");
  long n_rows = emu_obj->dimensions[0];
  long n_columns = emu_obj->dimensions[1];
  long row_stride = emu_obj->strides[0];
  long column_stride = emu_obj->strides[1];

  std::vector<std::string> keys;
  for (long i = 0; i < n_columns; ++i) {
    keys.push_back(jubatus::util::lang::lexical_cast<std::string>(i));
  }

  jubafvconv::datum d;
  for (long row_index = 0; row_index < n_rows; ++row_index) {
    char *p = emu_obj->data + (row_index * row_stride);
    if (dtype == DTYPE_F8) {
      double *v = (double*)p;
      for (long i = 0; i < n_columns; ++i) {
        d.num_values_.push_back(std::pair<std::string, double>(keys[i], v[i]));
      }
    } else if (dtype == DTYPE_F4) {
      float *v = (float*)p;
      for (long i = 0; i < n_columns; ++i) {
        d.num_values_.push_back(std::pair<std::string, double>(keys[i], v[i]));
      }
    } else if (dtype == DTYPE_I8) {
      int64_t *v = (int64_t*)p;
      for (long i = 0; i < n_columns; ++i) {
        d.num_values_.push_back(std::pair<std::string, double>(keys[i], (double)v[i]));
      }
    }
    out.push_back(d);
    d.num_values_.clear();
  }
}

static int64_t ReadInt(NumPyDataType dtype, char *p, int64_t idx)
{
    if (dtype == DTYPE_I4)
        return ((int32_t*)p)[idx];
    if (dtype == DTYPE_I8)
        return ((int64_t*)p)[idx];
    throw std::runtime_error("invalid dtype");
}

static double ReadDouble(NumPyDataType dtype, char *p, int64_t idx)
{
    if (dtype == DTYPE_I4)
        return (double)((int32_t*)p)[idx];
    if (dtype == DTYPE_I8)
        return (double)((int64_t*)p)[idx];
    if (dtype == DTYPE_F4)
        return ((float*)p)[idx];
    if (dtype == DTYPE_F8)
        return ((double*)p)[idx];
    throw std::runtime_error("invalid dtype");
}

static void CSRMatrixToDatum(PyObject *py_X, std::vector<jubafvconv::datum>& out)
{
    ScopedPyRef py_data(PyObject_GetAttrString(py_X, "data"));
    ScopedPyRef py_indices(PyObject_GetAttrString(py_X, "indices"));
    ScopedPyRef py_indptr(PyObject_GetAttrString(py_X, "indptr"));
    NumPyDataType dtype = GetNumPyDataType(py_data.get());
    NumPyDataType idx_dtype = GetNumPyDataType(py_indices.get());

    const NumpyArrayObject *emu_data = reinterpret_cast<const NumpyArrayObject*>(py_data.get());
    const NumpyArrayObject *emu_indices = reinterpret_cast<const NumpyArrayObject*>(py_indices.get());
    const NumpyArrayObject *emu_indptr = reinterpret_cast<const NumpyArrayObject*>(py_indptr.get());

    std::vector<std::string> keys;
    jubafvconv::datum d;
    for (int64_t row_index = 0; row_index < emu_indptr->dimensions[0] - 1; ++row_index) {
        int64_t start = ReadInt(idx_dtype, emu_indptr->data, row_index);
        int64_t end = ReadInt(idx_dtype, emu_indptr->data, row_index + 1);
        for (int64_t i = start; i < end; ++i) {
            int64_t col_index = ReadInt(idx_dtype, emu_indices->data, i);
            double v = ReadDouble(dtype, emu_data->data, i);
            while (keys.size() <= col_index) {
                keys.push_back(jubatus::util::lang::lexical_cast<std::string>(keys.size()));
            }
            d.num_values_.push_back(std::pair<std::string, double>(keys[col_index], v));
        }
        out.push_back(d);
        d.num_values_.clear();
    }
}

PyObject *ClassifierFit(ClassifierObject *self, PyObject *args)
{
    PyObject *py_X = NULL;
    PyObject *py_y = NULL;
    if (!PyArg_ParseTuple(args, "OO", &py_X, &py_y))
        return 0;
    if (!PyObject_TypeCheck(py_y, GetNDArrayType()))
        return 0;

    std::vector<jubafvconv::datum> datum_list;
    if (PyObject_TypeCheck(py_X, GetNDArrayType())) {
        NDArrayToDatum(py_X, datum_list);
    } else if (PyObject_TypeCheck(py_X, GetCSRMatrixType())) {
        CSRMatrixToDatum(py_X, datum_list);
    } else {
        return 0;
    }

    NumPyDataType dtype = GetNumPyDataType(py_y);
    if (dtype != DTYPE_I4 && dtype != DTYPE_I8)
      throw std::runtime_error("invalid y dtype");
    const NumpyArrayObject *emu_obj = reinterpret_cast<const NumpyArrayObject*>(py_y);
    if (emu_obj->nd != 1)
      throw std::runtime_error("invalid dimensions");
    if (emu_obj->dimensions[0] != datum_list.size())
      throw std::runtime_error("invalid y size");
    long stride = emu_obj->strides[0];

    for (size_t i = 0; i < datum_list.size(); ++i) {
      const jubafvconv::datum& d = datum_list[i];
      char *p = emu_obj->data + (i * stride);
      std::string label;
      if (dtype == DTYPE_I4) {
        label = jubatus::util::lang::lexical_cast<std::string>(*(int32_t*)p);
      } else if (dtype == DTYPE_I8) {
        label = jubatus::util::lang::lexical_cast<std::string>(*(int64_t*)p);
      }
      CATCH_CPP_EXCEPTION_AND_RETURN_NULL(self->handle->train(label, d));
    }

    Py_RETURN_NONE;
}

PyObject *ClassifierDecisionFunction(ClassifierObject *self, PyObject *py_X)
{
    std::vector<jubafvconv::datum> datum_list;
    if (PyObject_TypeCheck(py_X, GetNDArrayType())) {
        NDArrayToDatum(py_X, datum_list);
    } else if (PyObject_TypeCheck(py_X, GetCSRMatrixType())) {
        CSRMatrixToDatum(py_X, datum_list);
    } else {
        return 0;
    }

    PyObject *out = PyList_New(datum_list.size());
    for (size_t i = 0; i < datum_list.size(); ++i) {
        jubacore::classifier::classify_result ret;
        CATCH_CPP_EXCEPTION_AND_RETURN_NULL2(
            ret = self->handle->classify(datum_list[i]),
            Py_DECREF(out));
        PyObject *scores = PyTuple_New(ret.size());
        for (int j = 0; j < ret.size(); ++j) {
            PyTuple_SetItem(scores, atol(ret[j].label.c_str()),
                            PyFloat_FromDouble(ret[j].score));
        }
        PyList_SetItem(out, i, scores);
    }
    return out;
}

PyObject *ClassifierPredict(ClassifierObject *self, PyObject *py_X)
{
    std::vector<jubafvconv::datum> datum_list;
    if (PyObject_TypeCheck(py_X, GetNDArrayType())) {
        NDArrayToDatum(py_X, datum_list);
    } else if (PyObject_TypeCheck(py_X, GetCSRMatrixType())) {
        CSRMatrixToDatum(py_X, datum_list);
    } else {
        return 0;
    }

    PyObject *out = PyList_New(datum_list.size());
    for (size_t i = 0; i < datum_list.size(); ++i) {
        jubacore::classifier::classify_result ret;
        CATCH_CPP_EXCEPTION_AND_RETURN_NULL2(
            ret = self->handle->classify(datum_list[i]),
            Py_DECREF(out));
        std::string best_label;
        double best_score = -INFINITY;
        for (int j = 0; j < ret.size(); ++j) {
            if (ret[j].score > best_score) {
                best_score = ret[j].score;
                best_label = ret[j].label;
            }
        }
        PyList_SetItem(out, i, PyLong_FromLong(atol(best_label.c_str())));
    }
    return out;
}
