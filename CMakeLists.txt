cmake_minimum_required(VERSION 3.16)
project(hft_factor CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(HFT_COMMON_ROOT ${CMAKE_CURRENT_SOURCE_DIR}/third_party/hft-common)

set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/third_party/yaml-cpp
    ${CMAKE_CURRENT_BINARY_DIR}/yaml-cpp-build)

add_library(hft_factor_lib
    src/config_loader.cpp
    src/factor_pipeline_engine.cpp
)

target_include_directories(hft_factor_lib PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${HFT_COMMON_ROOT}/include
)

target_link_libraries(hft_factor_lib PUBLIC yaml-cpp pthread)

add_executable(hft_factor src/main.cpp)
target_link_libraries(hft_factor PRIVATE hft_factor_lib)
set_target_properties(hft_factor PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin
)
