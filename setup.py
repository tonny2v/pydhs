import os
import sys
from setuptools import setup, Extension

print('This module requires libboost-python-dev, libpython-dev')

# Detect Python version
py_major = sys.version_info.major
py_minor = sys.version_info.minor
py_version = f"{py_major}.{py_minor}"
py_version_nodot = f"{py_major}{py_minor}"

config = {
    'include_dirs': ['pydhs/header'],
    'library_dirs': [os.path.join(sys.prefix, 'lib')],
}

if os.environ.get('CPLUS_INCLUDE_PATH'):
    config['include_dirs'].extend([d for d in os.environ['CPLUS_INCLUDE_PATH'].split(':') if d])

if os.environ.get('LD_LIBRARY_PATH'):
    config['library_dirs'].extend([d for d in os.environ['LD_LIBRARY_PATH'].split(':') if d])

# Determine boost_python library name
# Use BOOST_PYTHON_LIB env var if available, otherwise try common names
boost_python_lib = os.environ.get('BOOST_PYTHON_LIB')
if not boost_python_lib:
    # Attempt to use versioned boost_python (common on Ubuntu/Debian)
    # or fall back to generic boost_python3 (common on Alpine/Arch)
    boost_python_lib = f'boost_python{py_version_nodot}'

libraries = [boost_python_lib, f'python{py_version}']

classifiers = [
    'Development Status :: 3 - Alpha',
    'Intended Audience :: Developers',
    'Topic :: Software Development :: Build Tools',
    'License :: OSI Approved :: MIT License',
    'Programming Language :: Python :: 3',
    f'Programming Language :: Python :: 3.{py_minor}',
]

# Get all .cpp files in pydhs/src
src_path = 'pydhs/src'
sources = [os.path.join(src_path, f) for f in os.listdir(src_path) if f.endswith('.cpp')]

dhs = Extension('dhs',
                sources=sources,
                define_macros=[('MAJOR_VERSION', '1'), ('MINOR_VERSION', '6')],
                extra_compile_args=['-std=c++11'],
                include_dirs=config['include_dirs'],
                library_dirs=config['library_dirs'],
                libraries=libraries)

setup(name='pydhs',
      classifiers=classifiers,
      license='MIT',
      version='1.6.8',
      description='Python wrapper of C++ Hyperpath algorithm implementation',
      long_description=open('README.md').read() if os.path.exists('README.md') else '',
      long_description_content_type='text/markdown',
      keywords='hyperpath',
      author='Jiangshan(Tonny) Ma',
      author_email='tonny.achilles@gmail.com',
      packages=['pydhs', 'pydhs.sample'],
      package_data={'pydhs': ['sample/*']},
      ext_modules=[dhs, ],
      python_requires='>=3.6')
