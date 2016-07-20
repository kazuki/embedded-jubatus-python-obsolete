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
    {"fit", (PyCFunction)ClassifierFit, METH_VARARGS, ""},
    {"decision_function", (PyCFunction)ClassifierDecisionFunction, METH_O, ""},
    {"predict", (PyCFunction)ClassifierPredict, METH_O, ""},
    {NULL}
};

static PyMethodDef AnomalyMethods[] = {
    {"add", (PyCFunction)AnomalyAdd, METH_O, ""},
    {"calc_score", (PyCFunction)AnomalyCalcScore, METH_O, ""},
    {"clear_row", (PyCFunction)AnomalyClearRow, METH_O, ""},
    {"update", (PyCFunction)AnomalyUpdate, METH_VARARGS, ""},
    {"overwrite", (PyCFunction)AnomalyOverwrite, METH_VARARGS, ""},
    {"get_all_rows", (PyCFunction)AnomalyGetAllRows, METH_NOARGS, ""},
    {"dump", (PyCFunction)AnomalyDump, METH_NOARGS, ""},
    {"load", (PyCFunction)AnomalyLoad, METH_O, ""},
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
    {"dump", (PyCFunction)RecommenderDump, METH_NOARGS, ""},
    {"load", (PyCFunction)RecommenderLoad, METH_O, ""},
    {"clear", (PyCFunction)CommonApiClear<RecommenderObject>, METH_NOARGS, ""},
    {"get_config", (PyCFunction)CommonApiGetConfig<RecommenderObject>, METH_NOARGS, ""},
    {NULL}
};

static PyMethodDef RegressionMethods[] = {
    {"train", (PyCFunction)RegressionTrain, METH_O, ""},
    {"estimate", (PyCFunction)RegressionEstimate, METH_O, ""},
    {"dump", (PyCFunction)RegressionDump, METH_NOARGS, ""},
    {"load", (PyCFunction)RegressionLoad, METH_O, ""},
    {"clear", (PyCFunction)CommonApiClear<RegressionObject>, METH_NOARGS, ""},
    {"get_config", (PyCFunction)CommonApiGetConfig<RegressionObject>, METH_NOARGS, ""},
    {NULL}
};

static PyMethodDef NearestNeighborMethods[] = {
    {"set_row", (PyCFunction)NearestNeighborSetRow, METH_VARARGS, ""},
    {"neighbor_row_from_id", (PyCFunction)NearestNeighborNeighborRowFromId, METH_VARARGS, ""},
    {"neighbor_row_from_datum", (PyCFunction)NearestNeighborNeighborRowFromDatum, METH_VARARGS, ""},
    {"similar_row_from_id", (PyCFunction)NearestNeighborSimilarRowFromId, METH_VARARGS, ""},
    {"similar_row_from_datum", (PyCFunction)NearestNeighborSimilarRowFromDatum, METH_VARARGS, ""},
    {"get_all_rows", (PyCFunction)NearestNeighborGetAllRows, METH_NOARGS, ""},
    {"dump", (PyCFunction)NearestNeighborDump, METH_NOARGS, ""},
    {"load", (PyCFunction)NearestNeighborLoad, METH_O, ""},
    {"clear", (PyCFunction)CommonApiClear<NearestNeighborObject>, METH_NOARGS, ""},
    {"get_config", (PyCFunction)CommonApiGetConfig<NearestNeighborObject>, METH_NOARGS, ""},
    {NULL}
};

static PyMethodDef ClusteringMethods[] = {
    {"push", (PyCFunction)ClusteringPush, METH_O, ""},
    {"get_revision", (PyCFunction)ClusteringGetRevision, METH_NOARGS, ""},
    {"get_core_members", (PyCFunction)ClusteringGetCoreMembers, METH_NOARGS, ""},
    {"get_k_center", (PyCFunction)ClusteringGetKCenter, METH_NOARGS, ""},
    {"get_nearest_center", (PyCFunction)ClusteringGetNearestCenter, METH_O, ""},
    {"get_nearest_members", (PyCFunction)ClusteringGetNearestMembers, METH_O, ""},
    {"dump", (PyCFunction)ClusteringDump, METH_NOARGS, ""},
    {"load", (PyCFunction)ClusteringLoad, METH_O, ""},
    {"clear", (PyCFunction)CommonApiClear<ClusteringObject>, METH_NOARGS, ""},
    {"get_config", (PyCFunction)CommonApiGetConfig<ClusteringObject>, METH_NOARGS, ""},
    {NULL}
};

static PyMethodDef BurstMethods[] = {
    {"dump", (PyCFunction)BurstDump, METH_NOARGS, ""},
    {"load", (PyCFunction)BurstLoad, METH_O, ""},
    {"clear", (PyCFunction)CommonApiClear<BurstObject>, METH_NOARGS, ""},
    {"get_config", (PyCFunction)CommonApiGetConfig<BurstObject>, METH_NOARGS, ""},
    {NULL}
};

static PyMethodDef BanditMethods[] = {
    {"dump", (PyCFunction)BanditDump, METH_NOARGS, ""},
    {"load", (PyCFunction)BanditLoad, METH_O, ""},
    {"clear", (PyCFunction)CommonApiClear<BanditObject>, METH_NOARGS, ""},
    {"get_config", (PyCFunction)CommonApiGetConfig<BanditObject>, METH_NOARGS, ""},
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
static PyTypeObject RegressionObjectType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "Regression",                 /* tp_name */
    sizeof(RegressionObjectType), /* tp_basicsize */
    0,                         /* tp_itemsize */
    (destructor)RegressionDealloc, /* tp_dealloc */
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
    "Regression",              /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    RegressionMethods,         /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)RegressionInit,  /* tp_init */
    0,                         /* tp_alloc */
    0,                         /* tp_new */
};
static PyTypeObject NearestNeighborObjectType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "NearestNeighbor",         /* tp_name */
    sizeof(NearestNeighborObjectType), /* tp_basicsize */
    0,                         /* tp_itemsize */
    (destructor)NearestNeighborDealloc, /* tp_dealloc */
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
    "NearestNeighbor",              /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    NearestNeighborMethods,    /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)NearestNeighborInit, /* tp_init */
    0,                         /* tp_alloc */
    0,                         /* tp_new */
};
static PyTypeObject ClusteringObjectType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "Clustering",              /* tp_name */
    sizeof(ClusteringObjectType), /* tp_basicsize */
    0,                         /* tp_itemsize */
    (destructor)ClusteringDealloc, /* tp_dealloc */
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
    "Clustering",              /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    ClusteringMethods,         /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)ClusteringInit,  /* tp_init */
    0,                         /* tp_alloc */
    0,                         /* tp_new */
};
static PyTypeObject BurstObjectType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "Burst",                   /* tp_name */
    sizeof(BurstObjectType),   /* tp_basicsize */
    0,                         /* tp_itemsize */
    (destructor)BurstDealloc,  /* tp_dealloc */
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
    "Burst",                   /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    BurstMethods,              /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)BurstInit,       /* tp_init */
    0,                         /* tp_alloc */
    0,                         /* tp_new */
};
static PyTypeObject BanditObjectType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "Bandit",                  /* tp_name */
    sizeof(BanditObjectType),  /* tp_basicsize */
    0,                         /* tp_itemsize */
    (destructor)BanditDealloc, /* tp_dealloc */
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
    "Bandit",                  /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    BanditMethods,             /* tp_methods */
    0,                         /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)BanditInit,      /* tp_init */
    0,                         /* tp_alloc */
    0,                         /* tp_new */
};
static PyTypeObject *_EmbeddedTypes[] = {
    &ClassifierObjectType,
    &AnomalyObjectType,
    &RecommenderObjectType,
    &RegressionObjectType,
    &NearestNeighborObjectType,
    &ClusteringObjectType,
    &BurstObjectType,
    &BanditObjectType,
    NULL
};

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
                           _EmbeddedTypes[i]->tp_name,
                           (PyObject*)_EmbeddedTypes[i]);
    }
    return MODINIT_RETURN_WRAP(m);
}
}
