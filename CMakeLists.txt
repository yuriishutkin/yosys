cmake_minimum_required(VERSION 3.10)

project(yosys)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules/)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

option(YOSYS_LINK_ABC "Whether to statically link to abc" OFF)

add_library(yosys SHARED)
target_include_directories(yosys PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(yosys PUBLIC _YOSYS_)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(yosys PUBLIC -Wno-deprecated-declarations)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  # target_compile_options(yosys PUBLIC)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  # target_compile_options(yosys PUBLIC)
endif()

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

bison_target(rtlil_parser
  ${CMAKE_SOURCE_DIR}/frontends/rtlil/rtlil_parser.y
  ${CMAKE_BINARY_DIR}/frontends/rtlil/rtlil_parser.tab.cc
  DEFINES_FILE ${CMAKE_BINARY_DIR}/frontends/rtlil/rtlil_parser.tab.hh
)

if(NOT DEFINED BISON_rtlil_parser_DEFINED)
  message(FATAL_ERROR "failed to generate rtlil parser")
endif()

flex_target(rtlil_lexer
  ${CMAKE_SOURCE_DIR}/frontends/rtlil/rtlil_lexer.l
  ${CMAKE_BINARY_DIR}/frontends/rtlil/rtlil_lexer.cc
  DEFINES_FILE ${CMAKE_BINARY_DIR}/frontends/rtlil/rtlil_lexer.hh
)

if(NOT DEFINED FLEX_rtlil_lexer_DEFINED)
  message(FATAL_ERROR "failed to generate rtlil lexer")
endif()

add_flex_bison_dependency(rtlil_lexer rtlil_parser)

bison_target(verilog_parser
  ${CMAKE_SOURCE_DIR}/frontends/verilog/verilog_parser.y
  ${CMAKE_BINARY_DIR}/frontends/verilog/verilog_parser.tab.cc
  DEFINES_FILE ${CMAKE_BINARY_DIR}/frontends/verilog/verilog_parser.tab.hh
)

if(NOT DEFINED BISON_verilog_parser_DEFINED)
  message(FATAL_ERROR "failed to generate verilog parser")
endif()

flex_target(verilog_lexer
  ${CMAKE_SOURCE_DIR}/frontends/verilog/verilog_lexer.l
  ${CMAKE_BINARY_DIR}/frontends/verilog/verilog_lexer.cc
  DEFINES_FILE ${CMAKE_BINARY_DIR}/frontends/verilog/verilog_lexer.hh
)

if(NOT DEFINED FLEX_verilog_lexer_DEFINED)
  message(FATAL_ERROR "failed to generate verilog lexer")
endif()

add_flex_bison_dependency(verilog_lexer verilog_parser)

target_include_directories(yosys PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

target_sources(yosys
  PRIVATE
  ${BISON_rtlil_parser_OUTPUT_SOURCE}
  ${FLEX_rtlil_lexer_OUTPUTS}
  ${BISON_verilog_parser_OUTPUT_SOURCE}
  ${FLEX_verilog_lexer_OUTPUTS}
)

find_package(Readline REQUIRED)
target_link_libraries(yosys PUBLIC Readline::Readline)
target_compile_definitions(yosys PUBLIC YOSYS_ENABLE_READLINE)

find_package(TCL)
if(NOT TCL_FOUND)
  message(FATAL_ERROR "could NOT find TCL")
endif()
target_link_libraries(yosys PUBLIC ${TCL_LIBRARY})
target_include_directories(yosys PUBLIC ${TCL_INCLUDE_PATH})
target_compile_definitions(yosys PUBLIC YOSYS_ENABLE_TCL)

if(NOT YOSYS_LINK_ABC AND NOT WIN32)
  find_package(ABC)
  target_compile_definitions(yosys PUBLIC ABCEXTERNAL="${ABC_EXECUTABLE}")
endif()
if(YOSYS_LINK_ABC OR WIN32 OR NOT ABC_FOUND)
  include(cmake/CPM.cmake)
  CPMAddPackage(
    NAME abc
    OPTIONS "ABC_NAMESPACE abc"
    GIT_REPOSITORY "https://github.com/rikushoney/abc_cmake.git"
    GIT_TAG "7ec838f8"
  )
  target_compile_options(abc PRIVATE -fPIC)
  target_link_libraries(yosys PRIVATE abc)
  target_compile_definitions(yosys PUBLIC YOSYS_LINK_ABC)
endif()
target_compile_definitions(yosys PUBLIC YOSYS_ENABLE_ABC)

find_package(ZLIB REQUIRED)
target_link_libraries(yosys PUBLIC ZLIB::ZLIB)
target_compile_definitions(yosys PUBLIC YOSYS_ENABLE_ZLIB)

add_subdirectory(backends)
add_subdirectory(frontends)
add_subdirectory(kernel)
add_subdirectory(libs)
add_subdirectory(passes)
add_subdirectory(techlibs)

add_executable(yosys_exe)
set_property(TARGET yosys_exe PROPERTY OUTPUT_NAME yosys)
target_sources(yosys_exe PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/kernel/driver.cc)
target_link_libraries(yosys_exe PRIVATE yosys)
