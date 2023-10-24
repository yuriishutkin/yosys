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

target_include_directories(yosys PUBLIC ${CMAKE_BINARY_DIR})

target_sources(yosys
  PRIVATE
  ${BISON_rtlil_parser_OUTPUT_SOURCE}
  ${FLEX_rtlil_lexer_OUTPUTS}
  ${BISON_verilog_parser_OUTPUT_SOURCE}
  ${FLEX_verilog_lexer_OUTPUTS}
)

add_executable(yosys_exe)
set_property(TARGET yosys_exe PROPERTY OUTPUT_NAME yosys)
target_sources(yosys_exe PRIVATE ${CMAKE_SOURCE_DIR}/kernel/driver.cc)
target_link_libraries(yosys_exe PRIVATE yosys)

add_subdirectory(backends)
add_subdirectory(frontends)
add_subdirectory(kernel)
add_subdirectory(libs)
add_subdirectory(passes)
add_subdirectory(techlibs)
