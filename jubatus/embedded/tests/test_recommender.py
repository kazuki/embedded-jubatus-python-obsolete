from nose.tools import assert_raises
from jubatus.embedded import Recommender
from jubatus.common import Datum

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
    x = Recommender(RECOMMENDER_CONFIG)
