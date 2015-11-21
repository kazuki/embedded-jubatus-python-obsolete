from nose.tools import assert_raises
from jubatus.embedded import Bandit
import json

BANDIT_CONFIG = {
    "method" : "epsilon_greedy",
    "parameter" : {
        "assume_unrewarded" : False,
        "epsilon" : 0.1
    }
}

def test_invalid_args():
    assert_raises(Exception, Bandit)
    assert_raises(Exception, Bandit, 'hoge')
    assert_raises(Exception, Bandit, {})
    assert_raises(Exception, Bandit, {'method': 'hoge'})

    invalid_config = dict(BANDIT_CONFIG)
    invalid_config['method'] = 'hoge'
    assert_raises(Exception, Bandit, invalid_config)

def test():
    x = Bandit(BANDIT_CONFIG)
    # TODO
    assert json.loads(x.get_config())
