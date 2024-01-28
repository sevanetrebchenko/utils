cmake_minimum_required(VERSION 3.3)
project(utils VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)

add_library("${PROJECT_NAME}"
    "${PROJECT_SOURCE_DIR}/src/string.cpp"
    "${PROJECT_SOURCE_DIR}/src/result.cpp"
)

target_include_directories("${PROJECT_NAME}" PUBLIC "${PROJECT_SOURCE_DIR}/include")
