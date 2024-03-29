from libc.stdint cimport uint64_t
from libcpp.string cimport string
from libcpp.vector cimport vector
from libcpp.pair cimport pair
from libcpp.map cimport map
from cython.operator cimport dereference
from cython.operator cimport preincrement

include 'defs.pyx'
IF NUMPY:
    cimport numpy as c_np

from _wrapper cimport _Anomaly
from _wrapper cimport _Bandit
from _wrapper cimport _Burst
from _wrapper cimport _Classifier
from _wrapper cimport _Clustering
from _wrapper cimport _NearestNeighbor
from _wrapper cimport _Recommender
from _wrapper cimport _Regression
from _wrapper cimport _Stat
from _wrapper cimport arm_info
from _wrapper cimport classify_result_elem
from _wrapper cimport datum
from _wrapper cimport keyword_params
from _wrapper cimport keyword_with_params
from _wrapper cimport lexical_cast

from jubatus.anomaly.types import IdWithScore as AnomalyIdWithScore
from jubatus.bandit.types import ArmInfo
from jubatus.burst.types import Batch
from jubatus.burst.types import Document
from jubatus.burst.types import KeywordWithParams
from jubatus.burst.types import Window
from jubatus.classifier.types import EstimateResult
from jubatus.classifier.types import LabeledDatum
from jubatus.clustering.types import WeightedDatum
from jubatus.common.datum import Datum
from jubatus.nearest_neighbor.types import IdWithScore as NNIdWithScore
from jubatus.recommender.types import IdWithScore as RecommenderIdWithScore
from jubatus.regression.types import ScoredDatum


class _JubatusBase(object):
    def __init__(self, config):
        import json
        if isinstance(config, str):
            # JSON parse test
            json.loads(config)
        else:
            config = json.dumps(config, sort_keys=True, indent=4)
        (self.get_config, self.save_bytes, self.load_bytes,
         self.clear, typ) = self._init(config.encode('utf8'))
        if str != bytes and isinstance(typ, bytes):
            typ = typ.decode('ascii')
        self._type = typ

    def _get_model_path(self, id_):
        host, port = '127.0.0.1', 0
        path = '/tmp/{host}_{port}_{type}_{id}.jubatus'.format(
            host=host, port=port, type=self._type, id=id_)
        return (path, {host: port})

    def load(self, id_):
        path, ret = self._get_model_path(id_)
        try:
            with open(path, 'rb') as f:
                self.load_bytes(f.read())
            return True
        except Exception:
            return False

    def save(self, id_):
        path, ret = self._get_model_path(id_)
        with open(path, 'wb') as f:
            f.write(self.save_bytes())
        return ret

    def get_status(self):
        raise RuntimeError

    def do_mix(self):
        raise RuntimeError

    def get_proxy_status(self):
        raise RuntimeError

    def get_name(self):
        raise RuntimeError

    def set_name(self, new_name):
        raise RuntimeError

    def get_client(self):
        raise RuntimeError

include 'types.pyx'
include 'anomaly.pyx'
include 'bandit.pyx'
include 'burst.pyx'
include 'classifier.pyx'
include 'clustering.pyx'
include 'nearest_neighbor.pyx'
include 'recommender.pyx'
include 'regression.pyx'
include 'stat.pyx'

class Anomaly(_JubatusBase, _AnomalyWrapper):
    pass

class Bandit(_JubatusBase, _BanditWrapper):
    pass

class Burst(_JubatusBase, _BurstWrapper):
    pass

class Classifier(_JubatusBase, _ClassifierWrapper):
    pass

class Clustering(_JubatusBase, _ClusteringWrapper):
    pass

class NearestNeighbor(_JubatusBase, _NearestNeighborWrapper):
    pass

class Recommender(_JubatusBase, _RecommenderWrapper):
    pass

class Regression(_JubatusBase, _RegressionWrapper):
    pass

class Stat(_JubatusBase, _StatWrapper):
    pass
