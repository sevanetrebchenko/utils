cmake_minimum_required(VERSION 3.3)
project(utils VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_library("${PROJECT_NAME}" INTERFACE
    "${PROJECT_SOURCE_DIR}/include/utils/result.hpp"
)
target_include_directories("${PROJECT_NAME}" INTERFACE "${PROJECT_SOURCE_DIR}/include")
