FROM alpine:latest

RUN apk add boost-python boost-dev musl-dev g++

ENV CPLUS_INCLUDE_PATH '/usr/include:/usr/include/python3.6m:/usr/local/include'

ENV LD_LIBRARY_PATH '/usr/lib:/usr/local/lib:/usr/local/lib64'

RUN apk add git py3-pip py3-numpy py-numpy-dev python3-dev cmake make

RUN rm /usr/bin/python && ln -s /usr/bin/python3.6 /usr/bin/python

# it has been deceprated and integrated into boost
#RUN mkdir /work && \
#    git clone https://github.com/ndarray/Boost.NumPy /work/Boost.Numpy && \
#    cd /work/Boost.Numpy && \
#    grep -n "project" CMakeLists.txt | awk -F ':' '{print $1}' | xargs -I{} \
#    sed '{}aset(CMAKE_CXX_FLAGS "-isystem /usr/include/c++/6.4.0")' -i CMakeLists.txt && \
#    cmake . \
#       -DPYTHON_INCLUDE_DIR=$(python3 -c "from distutils.sysconfig import get_python_inc; print(get_python_inc())")  \
#       -DPYTHON_LIBRARY=$(python3 -c "import distutils.sysconfig as sysconfig; print(sysconfig.get_config_var('LIBDIR'))")&& \
#    make -j4 && \
#    make install

COPY . /work
WORKDIR /work

RUN CFLAGS="-isystem /usr/include/c++/6.4.0" python3 setup.py build && python3 setup.py install