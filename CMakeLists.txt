cmake_minimum_required(VERSION 3.10)
project(pydhs LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find Python 3
find_package(Python3 REQUIRED COMPONENTS Development)

# Find NumPy
execute_process(
    COMMAND "${Python3_EXECUTABLE}" -c "import numpy; print(numpy.get_include())"
    OUTPUT_VARIABLE NUMPY_INCLUDE_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
)

# Find Boost with Python component
find_package(Boost REQUIRED COMPONENTS python${Python3_VERSION_MAJOR}${Python3_VERSION_MINOR})
if(NOT Boost_FOUND)
    find_package(Boost REQUIRED COMPONENTS python3)
endif()

include_directories(pydhs/header)
include_directories(${Python3_INCLUDE_DIRS})
include_directories(${NUMPY_INCLUDE_DIR})
include_directories(${Boost_INCLUDE_DIRS})

# Collect sources
file(GLOB SOURCES "pydhs/src/*.cpp")

# Define the python extension module
add_library(dhs MODULE ${SOURCES})

# Target properties
set_target_properties(dhs PROPERTIES
    PREFIX ""
    OUTPUT_NAME "dhs"
)

# Link libraries
target_link_libraries(dhs PRIVATE
    ${Python3_LIBRARIES}
    ${Boost_LIBRARIES}
)

# Install rules (optional but good practice)
install(TARGETS dhs DESTINATION .)
