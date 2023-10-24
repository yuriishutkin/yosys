cmake_minimum_required(VERSION 3.10)

project(yosys)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_library(yosys STATIC)
target_include_directories(yosys PUBLIC ${CMAKE_SOURCE_DIR})
target_compile_definitions(yosys PUBLIC _YOSYS_)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(yosys PUBLIC -Wno-deprecated-declarations)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  # target_compile_options(yosys PUBLIC)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  # target_compile_options(yosys PUBLIC)
endif()

add_executable(yosys_exe)
set_property(TARGET yosys_exe PROPERTY OUTPUT_NAME yosys)
target_sources(yosys_exe PRIVATE ${CMAKE_SOURCE_DIR}/kernel/driver.cc)
target_link_libraries(yosys_exe PRIVATE yosys)

add_subdirectory(backends)
add_subdirectory(frontends)
add_subdirectory(kernel)
add_subdirectory(passes)
