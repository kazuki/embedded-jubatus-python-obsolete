import json
import unittest

from embedded_jubatus import Clustering
from jubatus.clustering.types import WeightedDatum
from jubatus.common import Datum


CONFIG = {
    'method': 'kmeans',
    'parameter': {
        'k': 2,
        'bucket_size': 6,
        'bucket_length': 2,
        'compressed_bucket_size': 5,
        'compressor_method': 'compressive_kmeans',
        'bicriteria_base_size': 1,
        'forgetting_factor': 0.0,
        'forgetting_threshold': 0.5,
        'seed': 0,
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
            {'key': '*', 'type': 'space',
             'sample_weight': 'bin', 'global_weight': 'bin'}
        ]
    }
}


class TestClustering(unittest.TestCase):
    def test_invalid_configs(self):
        self.assertRaises(TypeError, Clustering)
        self.assertRaises(ValueError, Clustering, 'hoge')
        self.assertRaises(ValueError, Clustering, {})
        self.assertRaises(TypeError, Clustering, {'method': 'hoge'})
        self.assertRaises(RuntimeError, Clustering,
                          {'method': 'hoge', 'converter': {}})
        invalid_config = dict(CONFIG)
        invalid_config['method'] = 'hoge'
        self.assertRaises(RuntimeError, Clustering, invalid_config)

    def test(self):
        x = Clustering(CONFIG)
        self.assertEqual(0, x.get_revision())
        self.assertTrue(x.push([
            Datum({'x': 1.0}),
            Datum({'x': 0.9}),
            Datum({'x': 1.1}),
            Datum({'x': 5.0}),
            Datum({'x': 4.9}),
            Datum({'x': 5.1}),
        ]))
        self.assertEqual(1, x.get_revision())
        centers = x.get_k_center()
        self.assertTrue(isinstance(centers, list))
        self.assertEqual(2, len(centers))
        self.assertTrue(isinstance(centers[0], Datum))
        members = x.get_core_members()
        self.assertTrue(isinstance(members, list))
        self.assertEqual(2, len(members))
        self.assertTrue(isinstance(members[0], list))
        self.assertTrue(isinstance(members[0][0], WeightedDatum))
        c = x.get_nearest_center(Datum({'x': 1.05}))
        self.assertTrue(isinstance(c, Datum))
        self.assertTrue(c.num_values[0][1] >= 0.9 and
                        c.num_values[0][1] <= 1.1)
        c = x.get_nearest_members(Datum({'x': 1.05}))
        self.assertTrue(isinstance(c, list))
        self.assertTrue(isinstance(c[0], WeightedDatum))

        model = x.save_bytes()
        x = Clustering(CONFIG)
        x.load_bytes(model)

        self.assertEqual(CONFIG, json.loads(x.get_config()))
        self.assertEqual(1, x.get_revision())
        self.assertEqual(len(centers), len(x.get_k_center()))
