from nose.tools import assert_raises
from jubatus.embedded import Clustering
from jubatus.common import Datum
from jubatus.clustering.types import WeightedDatum
import json

CLUSTERING_CONFIG = {
    "method": "kmeans",
    "parameter" : {
        "k" : 2,
        "bucket_size" : 6,
        "bucket_length" : 2,
        "compressed_bucket_size" : 5,
        "compressor_method" : "compressive_kmeans",
        "bicriteria_base_size" : 1,
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
    assert x.get_revision() == 0
    assert x.push([
        Datum({'x': 1.0}),
        Datum({'x': 0.9}),
        Datum({'x': 1.1}),
        Datum({'x': 5.0}),
        Datum({'x': 4.9}),
        Datum({'x': 5.1}),
    ])
    assert x.get_revision() == 1
    centers = x.get_k_center()
    assert isinstance(centers, list) and len(centers) == 2
    assert isinstance(centers[0], Datum)
    members = x.get_core_members()
    assert isinstance(members, list) and len(members) == 2
    assert isinstance(members[0], list)
    assert isinstance(members[0][0], WeightedDatum)
    c = x.get_nearest_center(Datum({'x': 1.05}))
    assert isinstance(c, Datum)
    assert c.num_values[0][1] >= 0.9 and c.num_values[0][1] <= 1.1
    c = x.get_nearest_members(Datum({'x': 1.05}))
    assert isinstance(c, list)
    assert isinstance(c[0], WeightedDatum)
    assert json.loads(x.get_config())
