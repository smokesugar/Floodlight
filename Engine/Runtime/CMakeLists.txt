cmake_minimum_required(VERSION 3.10)

include(${CMAKE_SOURCE_DIR}/../Scripts/CMakeUtilities.txt)

project(Runtime)

set(INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/Core/Source ${CMAKE_SOURCE_DIR}/Dependencies/spdlog/include)
set(LINK_LIBS Core)

file(GLOB_RECURSE SOURCE_FILES Source/*.cpp Source/*.h)
SOURCE_GROUP_BY_FOLDER(${PROJECT_NAME})

add_executable(${PROJECT_NAME} WIN32 ${SOURCE_FILES})
include_directories(${INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${LINK_LIBS})
set_property(TARGET Runtime PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}")