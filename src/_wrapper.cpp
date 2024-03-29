#include <string>

#include <jubatus/core/common/jsonconfig.hpp>
#include <jubatus/core/fv_converter/converter_config.hpp>
#include <jubatus/core/storage/storage_factory.hpp>
#include <jubatus/core/storage/column_table.hpp>
#include <jubatus/core/anomaly/anomaly_factory.hpp>
#include <jubatus/core/classifier/classifier_factory.hpp>
#include <jubatus/core/clustering/clustering.hpp>
#include <jubatus/core/nearest_neighbor/nearest_neighbor_factory.hpp>
#include <jubatus/core/recommender/recommender_factory.hpp>
#include <jubatus/core/regression/regression_factory.hpp>

#include "_wrapper.h"

namespace jsonconfig = jubatus::core::common::jsonconfig;
using jubatus::core::fv_converter::converter_config;
using jubatus::core::fv_converter::make_fv_converter;
using jubatus::core::storage::storage_base;
using jubatus::core::storage::storage_factory;
using jubatus::util::lang::lexical_cast;

void parse_config(const std::string& config, std::string& method,
                  jsonconfig::config& params) {
    using jubatus::util::text::json::json;
    using jubatus::util::text::json::json_string;
    using jubatus::util::text::json::from_json;
    json config_json = lexical_cast<json>(config);
    json_string *method_value = (json_string*)config_json["method"].get();
    if (!method_value || method_value->type() != json::String)
        throw std::invalid_argument("invalid config (method)");
    method.assign(method_value->get());
    params = jsonconfig::config(config_json["parameter"]);
}

void parse_config(const std::string& config, std::string& method,
                  jsonconfig::config& params, converter_config& fvconv_config) {
    using jubatus::util::text::json::json;
    using jubatus::util::text::json::json_string;
    using jubatus::util::text::json::from_json;
    json config_json = lexical_cast<json>(config);
    json_string *method_value = (json_string*)config_json["method"].get();
    if (!method_value || method_value->type() != json::String)
        throw std::invalid_argument("invalid config (method)");
    method.assign(method_value->get());
    from_json(config_json["converter"], fvconv_config);
    params = jsonconfig::config(config_json["parameter"]);
}

_Classifier::_Classifier(const std::string& config) {
    using jubatus::core::classifier::classifier_factory;
    using jubatus::core::driver::classifier;
    std::string method;
    jsonconfig::config params;
    converter_config fvconv_config;
    parse_config(config, method, params, fvconv_config);
    handle.reset(new classifier(classifier_factory::create_classifier(
        method, params, storage_factory::create_storage("local")),
        make_fv_converter(fvconv_config, NULL)));
    this->config.assign(config);
}

void _Classifier::train(const std::string& label, const datum& d) {
    handle->train(label, d);
}

classify_result _Classifier::classify(const datum& d) {
    return handle->classify(d);
}

std::vector<std::pair<std::string, uint64_t> > _Classifier::get_labels() {
    using jubatus::core::classifier::labels_t;
    std::vector<std::pair<std::string, uint64_t> > ret;
    labels_t tmp = handle->get_labels();
    for (labels_t::iterator it = tmp.begin(); it != tmp.end(); ++it) {
        ret.push_back(std::pair<std::string, uint64_t>(it->first, it->second));
    }
    return ret;
}

bool _Classifier::set_label(const std::string& new_label) {
    return handle->set_label(new_label);
}

bool _Classifier::delete_label(const std::string& target_label) {
    return handle->delete_label(target_label);
}

_Regression::_Regression(const std::string& config) {
    using jubatus::core::driver::regression;
    using jubatus::core::regression::regression_factory;
    std::string method;
    jsonconfig::config params;
    converter_config fvconv_config;
    parse_config(config, method, params, fvconv_config);
    shared_ptr<storage_base> model = storage_factory::create_storage("local");
    handle.reset(new regression(model,
        regression_factory::create_regression(method, params, model),
        make_fv_converter(fvconv_config, NULL)));
    this->config.assign(config);
}

void _Regression::train(float score, const datum& d) {
    handle->train(std::pair<float, datum>(score, d));
}

float _Regression::estimate(const datum& d) {
    return handle->estimate(d);
}

_Recommender::_Recommender(const std::string& config) {
    using jubatus::core::driver::recommender;
    using jubatus::core::recommender::recommender_factory;
    std::string method;
    jsonconfig::config params;
    converter_config fvconv_config;
    parse_config(config, method, params, fvconv_config);
    std::string my_id;
    handle.reset(new recommender(
        recommender_factory::create_recommender(method, params, my_id),
        make_fv_converter(fvconv_config, NULL)));
    this->config.assign(config);
}

void _Recommender::clear_row(const std::string& id) {
    handle->clear_row(id);
}

void _Recommender::update_row(const std::string& id, const datum& d) {
    handle->update_row(id, d);
}

datum _Recommender::complete_row_from_id(const std::string& id) {
    return handle->complete_row_from_id(id);
}

datum _Recommender::complete_row_from_datum(const datum& d) {
    return handle->complete_row_from_datum(d);
}

std::vector<std::pair<std::string, float> > _Recommender::similar_row_from_id(const std::string& id, size_t ret_num) {
    return handle->similar_row_from_id(id, ret_num);
}

std::vector<std::pair<std::string, float> > _Recommender::similar_row_from_datum(const datum& d, size_t ret_num) {
    return handle->similar_row_from_datum(d, ret_num);
}

datum _Recommender::decode_row(const std::string& id) {
    return handle->decode_row(id);
}

std::vector<std::string> _Recommender::get_all_rows() {
    return handle->get_all_rows();
}

float _Recommender::calc_similarity(const datum& l, const datum& r) {
    return handle->calc_similarity(l, r);
}

float _Recommender::calc_l2norm(const datum& d) {
    return handle->calc_l2norm(d);
}

_NearestNeighbor::_NearestNeighbor(const std::string& config) {
    using jubatus::core::storage::column_table;
    using jubatus::core::driver::nearest_neighbor;
    using jubatus::core::nearest_neighbor::create_nearest_neighbor;
    std::string method;
    jsonconfig::config params;
    converter_config fvconv_config;
    parse_config(config, method, params, fvconv_config);
    std::string my_id;
    shared_ptr<column_table> table(new column_table);
    handle.reset(new nearest_neighbor(
        create_nearest_neighbor(method, params, table, my_id),
        make_fv_converter(fvconv_config, NULL)));
    this->config.assign(config);
}

void _NearestNeighbor::set_row(const std::string& id, const datum& d) {
    handle->set_row(id, d);
}

id_score_list_t _NearestNeighbor::neighbor_row_from_id(const std::string& id, size_t size) {
    return handle->neighbor_row_from_id(id, size);
}

id_score_list_t _NearestNeighbor::neighbor_row_from_datum(const datum& d, size_t size) {
    return handle->neighbor_row_from_datum(d, size);
}

id_score_list_t _NearestNeighbor::similar_row_from_id(const std::string& id, size_t size) {
    return handle->similar_row(id, size);
}

id_score_list_t _NearestNeighbor::similar_row_from_datum(const datum& d, size_t size) {
    return handle->similar_row(d, size);
}

std::vector<std::string> _NearestNeighbor::get_all_rows() {
    return handle->get_all_rows();
}

_Anomaly::_Anomaly(const std::string& config) : idgen(0) {
    using jubatus::core::driver::anomaly;
    using jubatus::core::anomaly::anomaly_factory;
    std::string method;
    jsonconfig::config params;
    converter_config fvconv_config;
    parse_config(config, method, params, fvconv_config);
    std::string my_id;
    handle.reset(new anomaly(
        anomaly_factory::create_anomaly(method, params, my_id),
        make_fv_converter(fvconv_config, NULL)));
    this->config.assign(config);
}

void _Anomaly::load(const std::string& data, const std::string& type, uint64_t version) {
    _Base<jubatus::core::driver::anomaly>::load(data, type, version);
    idgen = handle->find_max_int_id() + 1;
}

void _Anomaly::clear_row(const std::string& id) {
    handle->clear_row(id);
}

std::pair<std::string, float> _Anomaly::add(const datum& d) {
    std::string id = lexical_cast<std::string>(idgen++);
    return handle->add(id, d);
}

float _Anomaly::update(const std::string& id, const datum& d) {
    return handle->update(id, d);
}

float _Anomaly::overwrite(const std::string &id, const datum& d) {
    return handle->overwrite(id, d);
}

float _Anomaly::calc_score(const datum& d) const {
    return handle->calc_score(d);
}

std::vector<std::string> _Anomaly::get_all_rows() const {
    return handle->get_all_rows();
}

_Clustering::_Clustering(const std::string& config) {
    using jubatus::core::clustering::clustering;
    using jubatus::core::clustering::clustering_config;
    std::string method;
    jsonconfig::config params;
    converter_config fvconv_config;
    parse_config(config, method, params, fvconv_config);
    std::string my_id;
    clustering_config cluster_conf = jsonconfig::config_cast_check<clustering_config>(params);
    handle.reset(new jubatus::core::driver::clustering(
        shared_ptr<clustering>(
            new clustering(my_id, method, cluster_conf)),
        make_fv_converter(fvconv_config, NULL)));
    this->config.assign(config);
}

void _Clustering::push(const std::vector<datum>& points) {
    handle->push(points);
}

size_t _Clustering::get_revision() const {
    return handle->get_revision();
}

cluster_set _Clustering::get_core_members() const {
    return handle->get_core_members();
}

std::vector<datum> _Clustering::get_k_center() const {
    return handle->get_k_center();
}

datum _Clustering::get_nearest_center(const datum& d) const {
    return handle->get_nearest_center(d);
}

cluster_unit _Clustering::get_nearest_members(const datum& d) const {
    return handle->get_nearest_members(d);
}

_Burst::_Burst(const std::string& config) {
    using jubatus::core::burst::burst;
    using jubatus::core::burst::burst_options;
    std::string method;
    jsonconfig::config params;
    parse_config(config, method, params);
    burst_options opts = jsonconfig::config_cast_check<burst_options>(params);
    handle.reset(new jubatus::core::driver::burst(new burst(opts)));
    this->config.assign(config);
}

bool _Burst::add_document(const std::string& str, double pos) {
    return handle->add_document(str, pos);
}

_Burst::window to_window(const jubatus::core::burst::burst_result& x) {
    using jubatus::core::burst::batch_result;
    std::vector<_Burst::Batch> ret;
    double start_pos = x.get_start_pos();
    const std::vector<batch_result>& batches = x.get_batches();
    ret.reserve(batches.size());
    for (size_t i = 0; i < batches.size(); ++i) {
        _Burst::Batch b;
        b.all_data_count = batches[i].d;
        b.relevant_data_count = batches[i].r;
        b.burst_weight = batches[i].burst_weight;
        ret.push_back(b);
    }
    return std::make_pair(start_pos, ret);
}

std::map<std::string, _Burst::window>
to_window_map(const jubatus::core::burst::burst::result_map& x) {
    std::map<std::string, _Burst::window> result;
    for (jubatus::core::burst::burst::result_map::const_iterator iter = x.begin();
         iter != x.end(); ++iter) {
        result.insert(std::make_pair(iter->first, to_window(iter->second)));
    }
    return result;
}

_Burst::window _Burst::get_result(const std::string& keyword) const {
    return to_window(handle->get_result(keyword));
}

_Burst::window _Burst::get_result_at(const std::string& keyword, double pos) const {
    return to_window(handle->get_result_at(keyword, pos));
}

_Burst::window_map _Burst::get_all_bursted_results() const {
    return to_window_map(handle->get_all_bursted_results());
}

_Burst::window_map _Burst::get_all_bursted_results_at(double pos) const {
    return to_window_map(handle->get_all_bursted_results_at(pos));
}

_Burst::keyword_list _Burst::get_all_keywords() const {
    return handle->get_all_keywords();
}

bool _Burst::add_keyword(const std::string& keyword, const keyword_params& params) {
    return handle->add_keyword(keyword, params, true);
}

bool _Burst::remove_keyword(const std::string& keyword) {
    return handle->remove_keyword(keyword);
}

bool _Burst::remove_all_keywords() {
    return handle->remove_all_keywords();
}

void _Burst::calculate_results() {
    handle->calculate_results();
}

_Bandit::_Bandit(const std::string& config) {
    using jubatus::core::driver::bandit;
    std::string method;
    jsonconfig::config params;
    parse_config(config, method, params);
    handle.reset(new bandit(method, params));
    this->config.assign(config);
}

bool _Bandit::register_arm(const std::string& arm_id) {
    return handle->register_arm(arm_id);
}

bool _Bandit::delete_arm(const std::string& arm_id) {
    return handle->delete_arm(arm_id);
}

std::string _Bandit::select_arm(const std::string& player_id) {
    return handle->select_arm(player_id);
}

bool _Bandit::register_reward(const std::string& player_id, const std::string& arm_id, double reward) {
    return handle->register_reward(player_id, arm_id, reward);
}

std::map<std::string, jubatus::core::bandit::arm_info> _Bandit::get_arm_info(const std::string& player_id) const {
    using jubatus::core::bandit::arm_info_map;
    std::map<std::string, jubatus::core::bandit::arm_info> ret;
    arm_info_map r = handle->get_arm_info(player_id);
    for (arm_info_map::iterator it = r.begin(); it != r.end(); ++it) {
        ret[it->first] = it->second;
    }
    return ret;
}

bool _Bandit::reset(const std::string& player_id) {
    return handle->reset(player_id);
}

struct stat_serv_config {
  int32_t window_size;

  template<typename Ar>
  void serialize(Ar& ar) {
    ar & JUBA_MEMBER(window_size);
  }
};

_Stat::_Stat(const std::string& config) {
    using jubatus::core::stat::stat;
    using jubatus::util::text::json::json;
    jsonconfig::config conf_root(lexical_cast<json>(config));
    stat_serv_config conf =jsonconfig::config_cast_check<stat_serv_config>(conf_root);
    handle.reset(new jubatus::core::driver::stat(new stat(conf.window_size)));
    this->config.assign(config);
}

void _Stat::push(const std::string& key, double value) {
    handle->push(key, value);
}

double _Stat::sum(const std::string& key) const {
    return handle->sum(key);
}

double _Stat::stddev(const std::string& key) const {
    return handle->stddev(key);
}

double _Stat::max(const std::string& key) const {
    return handle->max(key);
}

double _Stat::min(const std::string& key) const {
    return handle->min(key);
}

double _Stat::entropy() const {
    return handle->entropy();
}

double _Stat::moment(const std::string& key, int degree, double center) const {
    return handle->moment(key, degree, center);
}
