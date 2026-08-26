cmake_minimum_required(VERSION 3.14)
cmake_policy(SET CMP0100 NEW)

project(Omnidome)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_CXX_STANDARD 14)

add_compile_definitions(_USE_MATH_DEFINES=1)

include(${CMAKE_SOURCE_DIR}/cmake/version.cmake)

omni_set_version(1 1 0)

add_subdirectory(lib)
add_subdirectory(widgetlib)
add_subdirectory(app)
add_subdirectory(plugins)
add_subdirectory(test)

