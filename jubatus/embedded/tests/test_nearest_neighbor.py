from nose.tools import assert_raises
from jubatus.embedded import NearestNeighbor
import json

NEARESTNEIGHBOR_CONFIG = {
    "method": "lsh",
    "parameter": {
        "hash_num" : 64
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
    assert_raises(Exception, NearestNeighbor)
    assert_raises(Exception, NearestNeighbor, 'hoge')
    assert_raises(Exception, NearestNeighbor, {})
    assert_raises(Exception, NearestNeighbor, {'method': 'hoge'})
    assert_raises(Exception, NearestNeighbor, {'method': 'hoge', 'converter': {}})

    invalid_config = dict(NEARESTNEIGHBOR_CONFIG)
    invalid_config['method'] = 'hoge'
    assert_raises(Exception, NearestNeighbor, invalid_config)

def test():
    x = NearestNeighbor(NEARESTNEIGHBOR_CONFIG)
    # TODO
    assert json.loads(x.get_config())
