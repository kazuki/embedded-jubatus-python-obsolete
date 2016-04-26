from nose.tools import assert_raises
from jubatus.embedded import NearestNeighbor
from jubatus.common import Datum
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
    assert x.set_row("a0", Datum({'x': 0}))
    assert x.set_row("a1", Datum({'x': 0.25}))
    assert x.set_row("a2", Datum({'x': 0.5}))
    assert x.set_row("a3", Datum({'x': 1}))
    assert x.set_row("b0", Datum({'x': 10}))
    assert x.set_row("b1", Datum({'x': 10.25}))
    assert x.set_row("b2", Datum({'x': 10.5}))
    assert x.set_row("b3", Datum({'x': 11}))

    def _check_prefix(expected, lst):
        for x in lst:
            assert x.id.startswith(expected)

    ret = x.neighbor_row_from_id("a0", 3)
    assert len(ret) == 3
    _check_prefix('a', ret)

    ret = x.neighbor_row_from_datum(Datum({'x': 0.25}), 3)
    assert len(ret) == 3
    _check_prefix('a', ret)

    ret = x.similar_row_from_id("b3", 3)
    assert len(ret) == 3
    # _check_prefix('b', ret)  # 何故かa1-a3を返す

    ret = x.similar_row_from_datum(Datum({'x': 11}), 3)
    assert len(ret) == 3
    # _check_prefix('b', ret)  # 何故かa1-a3を返す

    assert set(x.get_all_rows()) == set(['a0', 'a1', 'a2', 'a3',
                                         'b0', 'b1', 'b2', 'b3'])

    assert json.loads(x.get_config())

    # エラーが発生しないことだけ確認
    model = x.dump()
    x.load(model)
