from nose.tools import assert_raises
from jubatus.embedded import Regression
from jubatus.common import Datum
from jubatus.regression.types import ScoredDatum
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
    assert x.train([
        ScoredDatum(0.0, Datum({'x': 1.0})),
        ScoredDatum(1.0, Datum({'x': 2.0})),
        ScoredDatum(2.0, Datum({'x': 4.0})),
        ScoredDatum(3.0, Datum({'x': 8.0})),
        ScoredDatum(4.0, Datum({'x': 16.0})),
    ]) == 5
    ret = x.estimate([
        Datum({'x': 32.0}),
        Datum({'x': 1.5}),
    ])
    assert len(ret) == 2
    assert ret[0] >= 8.0 and ret[0] < 9.0
    assert ret[1] >= 0.0 and ret[1] < 1.0
    assert json.loads(x.get_config())

    # エラーが発生しないことだけ確認
    model = x.dump()
    x.load(model)
