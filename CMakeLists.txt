cmake_minimum_required(VERSION 3.10)

project(yosys)

include(${PROJECT_SOURCE_DIR}/cmake/CPM.cmake)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules/)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

option(YOSYS_LINK_ABC "Whether to statically link to abc" OFF)
option(SMALL "Build small package" ON)
option(STATIC_LIB "Build static lib" OFF)

if(STATIC_LIB)
add_library(yosys)
else()
add_library(yosys SHARED)
endif()

target_include_directories(yosys PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_definitions(yosys PUBLIC _YOSYS_)

if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  target_compile_options(yosys PUBLIC -Wno-deprecated-declarations)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  # target_compile_options(yosys PUBLIC)
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  target_compile_options(yosys PUBLIC /wd4244 /wd4267)
  target_compile_definitions(yosys
    PUBLIC UNICODE _CRT_SECURE_NO_WARNINGS
    PRIVATE YOSYS_DLL_EXPORT
  )
  set_target_properties(yosys PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
endif()

if(WIN32)
  set(WFB_BUILD_DIR ${CMAKE_BINARY_DIR}/winflexbison)
  CPMAddPackage(
	NAME winflexbison
    GIT_REPOSITORY "https://github.com/lexxmark/winflexbison.git"
    GIT_TAG "300f48b"
    DOWNLOAD_ONLY
  )
  
  message(STATUS "Configuring winflexbison")
  
  execute_process(
    COMMAND ${CMAKE_COMMAND}
            -S ${winflexbison_SOURCE_DIR}
            -G ${CMAKE_GENERATOR}
            -DCMAKE_BUILD_TYPE=Release
            -B ${WFB_BUILD_DIR}
    OUTPUT_FILE wfb_cmake.out
    ERROR_FILE wfb_cmake.err
  )
  
  message(STATUS "Building winflexbison")
  
  execute_process(
    COMMAND ${CMAKE_COMMAND} --build ${WFB_BUILD_DIR} --config Release
    OUTPUT_FILE wfb_build.out
    ERROR_FILE wfb_build.err
  )
  find_program(BISON_EXECUTABLE
    NAMES win_bison
    HINTS ${winflexbison_SOURCE_DIR}/bin/Release
    DOC "path to the bison executable"
    REQUIRED
  )
  find_program(FLEX_EXECUTABLE
    NAMES win_flex
    HINTS ${winflexbison_SOURCE_DIR}/bin/Release
    DOC "path to the flex executable"
    REQUIRED
  )
endif()

find_package(BISON REQUIRED)
find_package(FLEX REQUIRED)

bison_target(rtlil_parser
  ${PROJECT_SOURCE_DIR}/frontends/rtlil/rtlil_parser.y
  ${CMAKE_BINARY_DIR}/frontends/rtlil/rtlil_parser.tab.cc
  DEFINES_FILE ${CMAKE_BINARY_DIR}/frontends/rtlil/rtlil_parser.tab.hh
)

if(NOT DEFINED BISON_rtlil_parser_DEFINED)
  message(FATAL_ERROR "failed to generate rtlil parser")
endif()

flex_target(rtlil_lexer
  ${PROJECT_SOURCE_DIR}/frontends/rtlil/rtlil_lexer.l
  ${CMAKE_BINARY_DIR}/frontends/rtlil/rtlil_lexer.cc
  DEFINES_FILE ${CMAKE_BINARY_DIR}/frontends/rtlil/rtlil_lexer.hh
)

if(NOT DEFINED FLEX_rtlil_lexer_DEFINED)
  message(FATAL_ERROR "failed to generate rtlil lexer")
endif()

add_flex_bison_dependency(rtlil_lexer rtlil_parser)

bison_target(verilog_parser
  ${PROJECT_SOURCE_DIR}/frontends/verilog/verilog_parser.y
  ${CMAKE_BINARY_DIR}/frontends/verilog/verilog_parser.tab.cc
  DEFINES_FILE ${CMAKE_BINARY_DIR}/frontends/verilog/verilog_parser.tab.hh
)

if(NOT DEFINED BISON_verilog_parser_DEFINED)
  message(FATAL_ERROR "failed to generate verilog parser")
endif()

flex_target(verilog_lexer
  ${PROJECT_SOURCE_DIR}/frontends/verilog/verilog_lexer.l
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

find_package(Readline)
if(Readline_FOUND)
  target_link_libraries(yosys PUBLIC Readline::Readline)
  target_compile_definitions(yosys PUBLIC YOSYS_ENABLE_READLINE)
endif()

if(NOT WIN32)
  find_package(TCL)
  if(NOT TCL_FOUND)
    message(FATAL_ERROR "could NOT find TCL")
  endif()
  target_link_libraries(yosys PUBLIC ${TCL_LIBRARY})
  target_include_directories(yosys PUBLIC ${TCL_INCLUDE_PATH})
  target_compile_definitions(yosys PUBLIC YOSYS_ENABLE_TCL)
endif()

if(NOT SMALL)
if(NOT YOSYS_LINK_ABC AND NOT WIN32)
  find_package(ABC)
  target_compile_definitions(yosys PUBLIC ABCEXTERNAL="${ABC_EXECUTABLE}")
endif()
if(YOSYS_LINK_ABC OR WIN32 OR NOT ABC_FOUND)
  CPMAddPackage(
    NAME abc
    OPTIONS "ABC_NAMESPACE abc"
    GIT_REPOSITORY "https://github.com/rikushoney/abc_cmake.git"
    GIT_TAG "64258d84"
  )
  if(NOT WIN32)
    target_compile_options(abc PRIVATE -fPIC)
  endif()
  target_link_libraries(yosys PRIVATE abc)
  target_compile_definitions(yosys PUBLIC YOSYS_LINK_ABC)
endif()
target_compile_definitions(yosys PUBLIC YOSYS_ENABLE_ABC)
endif()

find_package(ZLIB)
if(ZLIB_FOUND)
  target_link_libraries(yosys PUBLIC ZLIB::ZLIB)
  target_compile_definitions(yosys PUBLIC YOSYS_ENABLE_ZLIB)
else()
  CPMAddPackage(
    NAME zlib
    GIT_REPOSITORY "https://github.com/madler/zlib.git"
    GIT_TAG "v1.3.1"
  )

  if(zlib_ADDED)
    message(STATUS "zlib package added: ${zlib_SOURCE_DIR}")
    target_link_libraries(yosys PUBLIC zlib)
    target_include_directories(yosys PUBLIC ${zlib_SOURCE_DIR})
    target_compile_definitions(yosys PUBLIC YOSYS_ENABLE_ZLIB)
  endif()
endif()

function(include_all_subdirs)
    # Add subdirectories containing CMakeLists.txt files
    file(GLOB subdirectories LIST_DIRECTORIES true ${CMAKE_CURRENT_SOURCE_DIR}/*)
    foreach(subdirectory ${subdirectories})
        if(IS_DIRECTORY ${subdirectory})
            if(EXISTS ${subdirectory}/CMakeLists.txt)
                add_subdirectory(${subdirectory})
            endif()
        endif()
    endforeach()
endfunction()

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

target_sources(yosys
  PUBLIC
  FILE_SET HEADERS
  BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR}
  FILES
  kernel/binding.h
  kernel/cellaigs.h
  kernel/celledges.h
  kernel/celltypes.h
  kernel/consteval.h
  kernel/constids.inc
  kernel/cost.h
  kernel/ff.h
  kernel/ffinit.h
  kernel/ffmerge.h
  kernel/fmt.h
  kernel/fstdata.h
  kernel/hashlib.h
  kernel/json.h
  kernel/log.h
  kernel/macc.h
  kernel/modtools.h
  kernel/mem.h
  kernel/qcsat.h
  kernel/register.h
  kernel/rtlil.h
  kernel/satgen.h
  kernel/sigtools.h
  kernel/timinginfo.h
  kernel/utils.h
  kernel/yosys.h
  kernel/yw.h
  libs/ezsat/ezsat.h
  libs/ezsat/ezminisat.h
  libs/fst/fstapi.h
  libs/sha1/sha1.h
  libs/json11/json11.hpp
  passes/fsm/fsmdata.h
  frontends/ast/ast.h
  frontends/ast/ast_binding.h
  frontends/blif/blifparse.h
  backends/rtlil/rtlil_backend.h
  backends/cxxrtl/runtime/cxxrtl/cxxrtl.h
  backends/cxxrtl/runtime/cxxrtl/cxxrtl_vcd.h
  backends/cxxrtl/runtime/cxxrtl/capi/cxxrtl_capi.cc
  backends/cxxrtl/runtime/cxxrtl/capi/cxxrtl_capi.h
  backends/cxxrtl/runtime/cxxrtl/capi/cxxrtl_capi_vcd.cc
  backends/cxxrtl/runtime/cxxrtl/capi/cxxrtl_capi_vcd.h
)

install(TARGETS yosys yosys_exe FILE_SET HEADERS)
