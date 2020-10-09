cmake_minimum_required(VERSION 3.13)
project(ZestCompiler)

if (NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin)

execute_process(
  COMMAND llvm-config --libs
  OUTPUT_VARIABLE LLVM_LIBS
  OUTPUT_STRIP_TRAILING_WHITESPACE)

execute_process(
  COMMAND llvm-config --includedir
  OUTPUT_VARIABLE LLVM_INCDIR
  OUTPUT_STRIP_TRAILING_WHITESPACE)

add_subdirectory(src)
