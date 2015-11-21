from nose.tools import assert_raises
from jubatus.embedded import Classifier
from jubatus.common import Datum
import json

CLASSIFIER_CONFIG = {
    "method": "perceptron",
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
    },
    "parameter": {}
}

def test_classifier_invalid_args():
    assert_raises(Exception, Classifier)
    assert_raises(Exception, Classifier, 'hoge')
    assert_raises(Exception, Classifier, {})
    assert_raises(Exception, Classifier, {'method': 'hoge'})
    assert_raises(Exception, Classifier, {'method': 'hoge', 'converter': {}})

    invalid_config = dict(CLASSIFIER_CONFIG)
    invalid_config['method'] = 'hoge'
    assert_raises(Exception, Classifier, invalid_config)

def test_classifier_num():
    x = Classifier(CLASSIFIER_CONFIG);
    x.train([
        (u'Y', Datum({'x': 1})),
        (u'N', Datum({'x': -1})),
    ])
    y = x.classify([
        Datum({'x': 1}),
        Datum({'x': -1})
    ])
    assert [list(sorted(z, key=lambda x:x.score, reverse=True))[0].label
            for z in y] == ['Y', 'N']
    assert sorted(x.get_labels()) == [u'N', u'Y']

    model = x.dump()

    x.clear()
    assert len(x.get_labels()) == 0
    x.set_label(u'Y')
    x.set_label(u'N')
    assert sorted(x.get_labels()) == [u'N', u'Y']
    x.delete_label(u'Y')
    assert x.get_labels() == [u'N']

    x.clear()
    assert len(x.get_labels()) == 0

    x.load(model)
    y = x.classify([
        Datum({'x': 1}),
        Datum({'x': -1})
    ])
    assert [list(sorted(z, key=lambda x:x.score, reverse=True))[0].label
            for z in y] == ['Y', 'N']
    assert sorted(x.get_labels()) == [u'N', u'Y']
    assert json.loads(x.get_config())

def test_classifier_str():
    x = Classifier(CLASSIFIER_CONFIG);
    x.train([
        (u'Y', Datum({'x': u'y'})),
        (u'N', Datum({'x': u'n'})),
    ])
    y = x.classify([
        Datum({'x': 'y'}),
        Datum({'x': 'n'})
    ])
    assert [list(sorted(z, key=lambda x:x.score, reverse=True))[0].label
            for z in y] == ['Y', 'N']
