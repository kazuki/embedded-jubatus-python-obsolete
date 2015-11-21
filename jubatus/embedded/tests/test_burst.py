from nose.tools import assert_raises
from jubatus.embedded import Burst
import json

BURST_CONFIG = {
    "method": "burst",
    "parameter" : {
        "window_batch_size": 5,
        "batch_interval": 10,
        "max_reuse_batch_num": 5,
        "costcut_threshold": -1,
        "result_window_rotate_size": 5
    }
}

def test_invalid_args():
    assert_raises(Exception, Burst)
    assert_raises(Exception, Burst, 'hoge')
    assert_raises(Exception, Burst, {})
    assert_raises(Exception, Burst, {'method': 'hoge'})

    invalid_config = dict(BURST_CONFIG)
    invalid_config['parameter'] = {}
    assert_raises(Exception, Burst, invalid_config)

def test():
    x = Burst(BURST_CONFIG)
    # TODO
    assert json.loads(x.get_config())
