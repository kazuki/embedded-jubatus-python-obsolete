from jubatus.embedded.native import Classifier
from jubatus.common import Datum

def test():
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
