cmake_minimum_required(VERSION 3.18)
project(pydhs LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(Python 3.10 REQUIRED COMPONENTS Interpreter Development.Module)

set(boost_python_component "python${Python_VERSION_MAJOR}${Python_VERSION_MINOR}")
set(boost_numpy_component "numpy${Python_VERSION_MAJOR}${Python_VERSION_MINOR}")
find_package(Boost REQUIRED COMPONENTS ${boost_python_component} ${boost_numpy_component})

set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/pydhs/src)
add_library(dhs MODULE
    ${SRC_DIR}/algorithm.cpp
    ${SRC_DIR}/dijkstra.cpp
    ${SRC_DIR}/fibheap.cpp
    ${SRC_DIR}/hyperpath.cpp
    ${SRC_DIR}/radixheap.cpp
    ${SRC_DIR}/wrapper.cpp
)

target_include_directories(dhs
    PRIVATE
        ${Python_INCLUDE_DIRS}
        ${Boost_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}/pydhs/header
)

target_link_libraries(dhs
    PRIVATE
        Boost::${boost_python_component}
        Boost::${boost_numpy_component}
        Python::Module
)

target_compile_definitions(dhs
    PRIVATE
        MAJOR_VERSION=1
        MINOR_VERSION=6
)

set_target_properties(dhs PROPERTIES
    PREFIX ""
    OUTPUT_NAME "dhs"
)

install(TARGETS dhs
    LIBRARY DESTINATION pydhs
)
