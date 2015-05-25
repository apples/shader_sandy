cmake_minimum_required(VERSION 3.2)
project(shader_sandy)

find_package(GLM REQUIRED)
find_package(jsoncpp REQUIRED)

find_package(PkgConfig REQUIRED)
pkg_search_module(GLFW REQUIRED glfw3)

add_subdirectory(glad) # glad
add_subdirectory(lodepng) # lodepng

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp)
add_executable(shader_sandy ${SOURCE_FILES})
set_property(TARGET shader_sandy PROPERTY CXX_STANDARD 14)
target_include_directories(shader_sandy PUBLIC ${GLFW_INCLUDE_DIRS} ${JSONCPP_INCLUDE_DIRS})
target_link_libraries(shader_sandy glad lodepng ${GLFW_LIBRARIES} ${JSONCPP_LIBRARIES})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
