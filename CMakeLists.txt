cmake_minimum_required(VERSION 3.13.4)
project(libmemintercept)

set(CMAKE_CXX_STANDARD 17)

add_library(libmemintercept OBJECT
    src/intercept.cpp
    include/libmemintercept.h
  )
target_include_directories(libmemintercept PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_link_libraries(libmemintercept PUBLIC dl)

add_subdirectory(examples)

enable_testing()
add_subdirectory(test)
