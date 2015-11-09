from setuptools import setup, find_packages, Extension

setup(name='embedded-jubatus',
      version='1.0',
      packages=[
          'jubatus/embedded',
          'jubatus/embedded/tests'
      ],
      install_requires=[
          'jubatus'
      ],
      ext_modules=[
          Extension('jubatus.embedded.native', [
              'native/lib.cxx',
              'native/classifier.cxx'
          ], include_dirs=['native'], libraries=['jubatus_core'])
      ]
)
