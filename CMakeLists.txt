cmake_minimum_required(VERSION 3.16)
project(BiteAppleFeedSys LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Python COMPONENTS Interpreter Development.Module REQUIRED)
execute_process(
    COMMAND "${Python_EXECUTABLE}" -m pybind11 --cmakedir
    OUTPUT_VARIABLE PYBIND11_CMAKE_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)
if(PYBIND11_CMAKE_DIR)
    set(pybind11_DIR "${PYBIND11_CMAKE_DIR}" CACHE PATH "pybind11 CMake config directory")
endif()
find_package(pybind11 CONFIG REQUIRED)

# List of common source files for both executable and library
set(COMMON_SOURCES
    src/DataStore.cpp
    src/Graph.cpp
    src/HashMap.cpp
    src/Heap.cpp
    src/InteractionManager.cpp
    src/Interation.cpp
    src/Item.cpp
    src/Queue.cpp
    src/RecommendationEngine.cpp
    src/User.cpp
    src/Vector.cpp
)

# Build the pybind11 Python module
pybind11_add_module(biteapple_core
    src/bindings/biteapple_core.cpp
    ${COMMON_SOURCES}
)

target_include_directories(biteapple_core PRIVATE src)
