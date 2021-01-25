cmake_minimum_required(VERSION 3.10)

project(Floodlight)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)
add_definitions(/MP)

add_subdirectory(Core)
add_subdirectory(Runtime)