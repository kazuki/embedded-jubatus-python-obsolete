#ifndef EMBEDDED_JUBATUS_PYTHON_HPP
#define EMBEDDED_JUBATUS_PYTHON_HPP

#include <Python.h>
#include <jubatus/core/common/jsonconfig.hpp>
#include <jubatus/core/fv_converter/converter_config.hpp>
#include <jubatus/core/fv_converter/datum.hpp>
#include <jubatus/core/driver/anomaly.hpp>
#include <jubatus/core/driver/classifier.hpp>
#include <jubatus/core/driver/nearest_neighbor.hpp>
#include <jubatus/core/driver/recommender.hpp>
#include <jubatus/core/driver/regression.hpp>
#include <jubatus/core/driver/clustering.hpp>
#include <jubatus/core/driver/burst.hpp>
#include <jubatus/core/driver/bandit.hpp>
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
typedef struct {
    PyObject_HEAD;
    shared_ptr<jubadriver::regression> handle;
    shared_ptr<std::string> config;
} RegressionObject;
typedef struct {
    PyObject_HEAD;
    shared_ptr<jubadriver::nearest_neighbor> handle;
    shared_ptr<std::string> config;
} NearestNeighborObject;
typedef struct {
    PyObject_HEAD;
    shared_ptr<jubadriver::clustering> handle;
    shared_ptr<std::string> config;
} ClusteringObject;
typedef struct {
    PyObject_HEAD;
    shared_ptr<jubadriver::burst> handle;
    shared_ptr<std::string> config;
} BurstObject;
typedef struct {
    PyObject_HEAD;
    shared_ptr<jubadriver::bandit> handle;
    shared_ptr<std::string> config;
} BanditObject;

template<typename T>
PyObject *CommonApiClear(T *self, PyObject *args) {
    self->handle->clear();
    Py_RETURN_NONE;
}
template<typename T>
PyObject *CommonApiGetConfig(T *self, PyObject *args) {
    const std::string &str = *(self->config);
    return PyUnicode_DecodeUTF8(str.data(), str.size(), NULL);
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

int AnomalyInit(AnomalyObject *self, PyObject *args, PyObject *kwargs);
void AnomalyDealloc(AnomalyObject *self);
PyObject *AnomalyAdd(AnomalyObject *self, PyObject *args);
PyObject *AnomalyCalcScore(AnomalyObject *self, PyObject *args);
PyObject *AnomalyClearRow(AnomalyObject *self, PyObject *args);
PyObject *AnomalyUpdate(AnomalyObject *self, PyObject *args);
PyObject *AnomalyOverwrite(AnomalyObject *self, PyObject *args);
PyObject *AnomalyGetAllRows(AnomalyObject *self, PyObject *args);
PyObject *AnomalyDump(AnomalyObject *self, PyObject *args);
PyObject *AnomalyLoad(AnomalyObject *self, PyObject *args);

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
PyObject *RecommenderDump(RecommenderObject *self, PyObject *args);
PyObject *RecommenderLoad(RecommenderObject *self, PyObject *args);

int RegressionInit(RegressionObject *self, PyObject *args, PyObject *kwargs);
void RegressionDealloc(RegressionObject *self);
PyObject *RegressionTrain(RegressionObject *self, PyObject *args);
PyObject *RegressionEstimate(RegressionObject *self, PyObject *args);
PyObject *RegressionDump(RegressionObject *self, PyObject *args);
PyObject *RegressionLoad(RegressionObject *self, PyObject *args);

int NearestNeighborInit(NearestNeighborObject *self, PyObject *args, PyObject *kwargs);
void NearestNeighborDealloc(NearestNeighborObject *self);
PyObject *NearestNeighborSetRow(NearestNeighborObject *self, PyObject *args);
PyObject *NearestNeighborNeighborRowFromId(NearestNeighborObject *self, PyObject *args);
PyObject *NearestNeighborNeighborRowFromDatum(NearestNeighborObject *self, PyObject *args);
PyObject *NearestNeighborSimilarRowFromId(NearestNeighborObject *self, PyObject *args);
PyObject *NearestNeighborSimilarRowFromDatum(NearestNeighborObject *self, PyObject *args);
PyObject *NearestNeighborGetAllRows(NearestNeighborObject *self, PyObject *args);
PyObject *NearestNeighborDump(NearestNeighborObject *self, PyObject *args);
PyObject *NearestNeighborLoad(NearestNeighborObject *self, PyObject *args);

int ClusteringInit(ClusteringObject *self, PyObject *args, PyObject *kwargs);
void ClusteringDealloc(ClusteringObject *self);
PyObject *ClusteringPush(ClusteringObject *self, PyObject *args);
PyObject *ClusteringGetRevision(ClusteringObject *self, PyObject *args);
PyObject *ClusteringGetCoreMembers(ClusteringObject *self, PyObject *args);
PyObject *ClusteringGetKCenter(ClusteringObject *self, PyObject *args);
PyObject *ClusteringGetNearestCenter(ClusteringObject *self, PyObject *args);
PyObject *ClusteringGetNearestMembers(ClusteringObject *self, PyObject *args);
PyObject *ClusteringDump(ClusteringObject *self, PyObject *args);
PyObject *ClusteringLoad(ClusteringObject *self, PyObject *args);

int BurstInit(BurstObject *self, PyObject *args, PyObject *kwargs);
void BurstDealloc(BurstObject *self);
PyObject *BurstDump(BurstObject *self, PyObject *args);
PyObject *BurstLoad(BurstObject *self, PyObject *args);

int BanditInit(BanditObject *self, PyObject *args, PyObject *kwargs);
void BanditDealloc(BanditObject *self);
PyObject *BanditDump(BanditObject *self, PyObject *args);
PyObject *BanditLoad(BanditObject *self, PyObject *args);

#endif
