from nose.tools import assert_raises
from jubatus.embedded import Regression
import json

REGRESSION_CONFIG = {
    "method": "PA",
    "parameter": {
        "sensitivity": 0.1,
        "regularization_weight": 3.402823e+38
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
    assert_raises(Exception, Regression)
    assert_raises(Exception, Regression, 'hoge')
    assert_raises(Exception, Regression, {})
    assert_raises(Exception, Regression, {'method': 'hoge'})
    assert_raises(Exception, Regression, {'method': 'hoge', 'converter': {}})

    invalid_config = dict(REGRESSION_CONFIG)
    invalid_config['method'] = 'hoge'
    assert_raises(Exception, Regression, invalid_config)

def test():
    x = Regression(REGRESSION_CONFIG)
    # TODO
    assert json.loads(x.get_config())
