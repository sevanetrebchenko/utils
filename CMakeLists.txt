cmake_minimum_required(VERSION 3.3)
project(utils VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

add_library("${PROJECT_NAME}"
    "${PROJECT_SOURCE_DIR}/src/assert.cpp"
    "${PROJECT_SOURCE_DIR}/src/datetime.cpp"
    "${PROJECT_SOURCE_DIR}/src/events.cpp"
    "${PROJECT_SOURCE_DIR}/src/exceptions.cpp"
    "${PROJECT_SOURCE_DIR}/src/filesystem.cpp"
    "${PROJECT_SOURCE_DIR}/src/logging.cpp"
    "${PROJECT_SOURCE_DIR}/src/memory.cpp"
    "${PROJECT_SOURCE_DIR}/src/string.cpp"
)

target_include_directories("${PROJECT_NAME}" PUBLIC "${PROJECT_SOURCE_DIR}/include")
