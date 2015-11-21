from nose.tools import assert_raises
from jubatus.embedded import Clustering
import json

CLUSTERING_CONFIG = {
    "method": "kmeans",
    "parameter" : {
        "k" : 3,
        "bucket_size" : 1000,
        "bucket_length" : 2,
        "compressed_bucket_size" : 100,
        "compressor_method" : "compressive_kmeans",
        "bicriteria_base_size" : 10,
        "forgetting_factor" : 0.0,
        "forgetting_threshold" : 0.5,
        "seed": 0,
    },
    "converter": {
        "num_filter_types": {},
        "num_filter_rules": [],
        "string_filter_types": {},
        "string_filter_rules": [],
        "num_types": {},
        "num_rules": [
            {"key": "*", "type": "num"}
        ],
        "string_types": {},
        "string_rules": [
            {"key": "*", "type": "space", "sample_weight": "bin", "global_weight": "bin"}
        ]
    }
}

def test_invalid_args():
    assert_raises(Exception, Clustering)
    assert_raises(Exception, Clustering, 'hoge')
    assert_raises(Exception, Clustering, {})
    assert_raises(Exception, Clustering, {'method': 'hoge'})
    assert_raises(Exception, Clustering, {'method': 'hoge', 'converter': {}})

    invalid_config = dict(CLUSTERING_CONFIG)
    invalid_config['method'] = 'hoge'
    assert_raises(Exception, Clustering, invalid_config)

def test():
    x = Clustering(CLUSTERING_CONFIG)
    # TODO
    assert json.loads(x.get_config())
