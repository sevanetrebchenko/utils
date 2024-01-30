cmake_minimum_required(VERSION 3.3)
project(utils VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

add_executable("${PROJECT_NAME}"
    "${PROJECT_SOURCE_DIR}/src/string.cpp"
    "${PROJECT_SOURCE_DIR}/src/result.cpp"

    "${PROJECT_SOURCE_DIR}/src/main.cpp"
)

target_include_directories("${PROJECT_NAME}" PUBLIC "${PROJECT_SOURCE_DIR}/include")
