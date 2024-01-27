# `utils`

## Overview
`utils` is a collection of C++ helper modules, utility classes, and miscellaneous functionality intended to be shared between projects.

## Building
This project is built using CMake, and requires a compiler that supports C++17 (or higher). Use the following example to include `utils` into your project's build tree:
```cmake
project(YOUR_PROJECT_NAME ...)
...
add_subdirectory(lib/utils) # Path to utils root directory.
target_link_libraries(YOUR_PROJECT_NAME utils)
```

## Sample Usage
