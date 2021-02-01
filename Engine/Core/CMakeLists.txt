cmake_minimum_required(VERSION 3.10)

include(${CMAKE_SOURCE_DIR}/../Scripts/CMakeUtilities.txt)

project(Core)

set(INCLUDE_DIRS ${PROJECT_SOURCE_DIR}/Source ${CMAKE_SOURCE_DIR}/Dependencies/spdlog/include)
set(LINK_LIBS dxgi.lib d3d12.lib d3dcompiler.lib)

file(GLOB_RECURSE SOURCE_FILES Source/*.cpp Source/*.h)
SOURCE_GROUP_BY_FOLDER(${PROJECT_NAME})

add_library(${PROJECT_NAME} STATIC ${SOURCE_FILES})
include_directories(${INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${LINK_LIBS})