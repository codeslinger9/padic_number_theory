cmake_minimum_required(VERSION 3.19)

project(padic_numbers CXX)

#list(APPEND CMAKE_PREFIX_PATH "/opt/homebrew")
#message(STATUS "CMAKE_PREFIX_PATH : ${CMAKE_PREFIX_PATH}")

include_directories(/opt/homebrew/include)
link_directories(/opt/homebrew/lib)

include_directories(/opt/homebrew/Cellar/flint/3.1.3-p1/include/)
link_directories(/opt/homebrew/Cellar/flint/3.1.3-p1/lib)


message(STATUS "INCLUDE_DIRECTORIES : ${INCLUDE_DIRECTORIES}")
message(STATUS "LINK_LIBRARIES      : ${LINK_LIBRARIES}")

set(CMAKE_VERBOSE_MAKEFILE 1)


add_executable(test main.cpp)
add_executable(padic padic.cpp)


target_link_libraries(padic flint)
