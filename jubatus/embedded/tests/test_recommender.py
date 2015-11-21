from nose.tools import assert_raises
from jubatus.embedded import Recommender
from jubatus.common import Datum
from jubatus.recommender.types import IdWithScore
import json

RECOMMENDER_CONFIG = {
    'method': 'lsh',
    'parameter': {
        'hash_num': 512,
    },
    'converter': {
        'num_filter_types': {},
        'num_filter_rules': [],
        'string_filter_types': {},
        'string_filter_rules': [],
        'num_types': {},
        'num_rules': [
            {'key': '*', 'type': 'num'}
        ],
        'string_types': {},
        'string_rules': [
            {'key': '*', 'type': 'space', 'sample_weight': 'bin', 'global_weight': 'bin'}
        ]
    },
}

def test_invalid_args():
    assert_raises(Exception, Recommender)
    assert_raises(Exception, Recommender, 'hoge')
    assert_raises(Exception, Recommender, {})
    assert_raises(Exception, Recommender, {'method': 'hoge'})
    assert_raises(Exception, Recommender, {'method': 'hoge', 'converter': {}})

    invalid_config = dict(RECOMMENDER_CONFIG)
    invalid_config['method'] = 'hoge'
    assert_raises(Exception, Recommender, invalid_config)

def test():
    def _valid_result(r):
        assert isinstance(r, Datum)
        d = dict(r.num_values)
        assert d.get('x', None) is not None and d.get('y', None) is not None

    x = Recommender(RECOMMENDER_CONFIG)
    x.update_row('0', Datum({'x': 0.9, 'y': 4.9}))
    x.update_row('1', Datum({'x': 1, 'y': 5}))
    x.update_row('2', Datum({'x': 1.15, 'y': 5.15}))
    x.update_row('3', Datum({'x': 1.2, 'y': 5.1}))
    x.update_row('4', Datum({'x': 1.05}))
    _valid_result(x.complete_row_from_datum(Datum({'x': 1.1})))
    _valid_result(x.complete_row_from_id('4'))
    r = x.similar_row_from_id('2', 3)
    assert isinstance(r, list)
    assert isinstance(r[0], IdWithScore)
    r = x.similar_row_from_datum(Datum({'y': 5.05}), 3)
    assert isinstance(r, list)
    assert isinstance(r[0], IdWithScore)
    _valid_result(x.decode_row('0'))
    assert set(x.get_all_rows()) == set([str(i) for i in range(5)])
    r = x.calc_similarity(Datum({'x': 1}), Datum({'y': 5}))
    assert isinstance(r, float)
    r = x.calc_l2norm(Datum({'x': 1, 'y': 5}))
    assert isinstance(r, float)

    x.clear()
    assert len(x.get_all_rows()) == 0
    assert json.loads(x.get_config())
