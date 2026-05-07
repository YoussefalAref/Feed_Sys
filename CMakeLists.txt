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

add_executable(phase5_ads_demo
    src/main.cpp
    src/models/Item.cpp
    src/models/User.cpp
    src/models/Interaction.cpp
    src/models/InteractionManager.cpp
    src/data_structures/Heap.cpp
    src/data_structures/Queue.cpp
    src/data_structures/Graph.cpp
)

target_include_directories(phase5_ads_demo PRIVATE src)

pybind11_add_module(biteapple_core
    src/bindings/biteapple_core.cpp
    src/models/Item.cpp
    src/models/Interaction.cpp
    src/data_structures/Heap.cpp
    src/data_structures/Queue.cpp
    src/data_structures/Graph.cpp
)

target_include_directories(biteapple_core PRIVATE src)
