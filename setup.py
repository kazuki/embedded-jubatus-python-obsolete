from setuptools import setup, find_packages, Extension

setup(
    name='embedded-jubatus',
    version='1.0',
    packages=[
        'jubatus/embedded/tests'
    ],
    install_requires=[
        'jubatus'
    ],
    ext_modules=[
        Extension(
            'jubatus.embedded', [
                'native/lib.cxx',
                'native/helper.cxx',
                'native/anomaly.cxx',
                'native/classifier.cxx',
                'native/recommender.cxx',
                'native/regression.cxx',
                'native/nearest_neighbor.cxx',
                'native/clustering.cxx',
                'native/burst.cxx',
                'native/bandit.cxx',
            ],
            include_dirs=['native'],
            libraries=['jubatus_core', 'jubaserv_common']
        )
    ]
)
