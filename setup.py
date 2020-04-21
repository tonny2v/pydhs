import os

from setuptools import Extension, setup

print('This module requires libboost-python-dev, libpython-dev')
config = {}
if os.environ.get('CPLUS_INCLUDE_PATH'):
    config['include_path'] = os.environ['CPLUS_INCLUDE_PATH'].split(':')

if os.environ.get('LD_LIBRARY_PATH'):
    config['library_path'] = os.environ['LD_LIBRARY_PATH'].split(':')

config['include_path'] = config.get('include_path') + [
    'pydhs/header',
]

libraries = ['python3.7m', 'boost_python37-mt']

classifiers = [
    'Development Status :: 3 - Alpha',
    'Intended Audience :: Developers',
    'Topic :: Software Development :: Build Tools',
    'License :: OSI Approved :: MIT License',
    'Programming Language :: Python :: 3.7',
]

print('lib path', config['library_path'])

dhs = Extension('dhs',
                sources=[
                    'pydhs/src/' + i for i in os.listdir('pydhs/src')
                    if i.endswith('cpp')
                ],
                define_macros=[('MAJOR_VERSION', '1'), ('MINOR_VERSION', '6')],
                extra_compile_args=['-std=c++11'],
                include_dirs=config.get('include_path'),
                library_dirs=config.get('library_path'),
                libraries=libraries)

setup(name='pydhs',
      classifiers=classifiers,
      license='MIT',
      version='1.6.6',
      description='Python wrapper of C++ Hyperpath algorithm implementation',
      keywords='hyperpath',
      author='Jiangshan(Tonny) Ma',
      author_email='tonny.achilles@gmail.com',
      packages=['pydhs', 'pydhs.sample'],
      package_data={'pydhs': ['sample/*']},
      ext_modules=[
          dhs,
      ])
