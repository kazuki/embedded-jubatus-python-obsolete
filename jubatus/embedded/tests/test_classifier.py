from jubatus.embedded import Classifier
from jubatus.common import Datum

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
