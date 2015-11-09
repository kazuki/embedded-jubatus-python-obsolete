from jubatus.embedded import Classifier
from jubatus.common import Datum

def test_classifier_num():
    x = Classifier()
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
    x = Classifier()
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
