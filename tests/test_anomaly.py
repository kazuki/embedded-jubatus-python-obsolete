from nose.tools import assert_raises
from jubatus.embedded import Anomaly
from jubatus.common import Datum
import json

ANOMALY_CONFIG = {
    'method': 'lof',
    'parameter': {
        'nearest_neighbor_num': 3,
        'reverse_nearest_neighbor_num': 5,
        'method': 'lsh',
        'parameter': {
            'hash_num': 512,
        }
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

def test_anomaly_invalid_args():
    assert_raises(Exception, Anomaly)
    assert_raises(Exception, Anomaly, 'hoge')
    assert_raises(Exception, Anomaly, {})
    assert_raises(Exception, Anomaly, {'method': 'hoge'})
    assert_raises(Exception, Anomaly, {'method': 'hoge', 'converter': {}})

    invalid_config = dict(ANOMALY_CONFIG)
    invalid_config['method'] = 'hoge'
    assert_raises(Exception, Anomaly, invalid_config)

def test_anomaly():
    x = Anomaly(ANOMALY_CONFIG);
    assert x.add(Datum({'x': 0.1})).score == float('inf')  # 1発目はinf
    # 取り敢えずメソッドを呼び出してもエラーにならないことだけを確認
    x.add(Datum({'x': 0.101}))
    x.add(Datum({'x': 0.1001}))
    x.calc_score(Datum({'x': 0.1001}))

    x.clear()
    assert x.add(Datum({'x': 0.1})).score == float('inf')
    assert json.loads(x.get_config())

    p = x.add(Datum({'x': 0.2}))
    assert x.update(p.id, Datum({'x': 0.2})) == p.score
    assert x.overwrite(p.id, Datum({'x': 0.2})) == p.score
    assert len(x.get_all_rows()) == 2

    # エラーが発生しないことだけ確認
    model = x.dump()
    x.load(model)