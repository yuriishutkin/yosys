find_package(Python3 COMPONENTS Interpreter REQUIRED)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/simcells_help.inc
  COMMAND
    ${Python3_EXECUTABLE}
    ${CMAKE_CURRENT_SOURCE_DIR}/cellhelp.py
    ${CMAKE_CURRENT_SOURCE_DIR}/simcells.v
    ${CMAKE_CURRENT_BINARY_DIR}/simcells_help.inc
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/simcells.v
)

add_custom_command(
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/simlib_help.inc
  COMMAND
    ${Python3_EXECUTABLE}
    ${CMAKE_CURRENT_SOURCE_DIR}/cellhelp.py
    ${CMAKE_CURRENT_SOURCE_DIR}/simlib.v
    ${CMAKE_CURRENT_BINARY_DIR}/simlib_help.inc
  DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/simlib.v
)

add_custom_target(
  generate_cellhelp
  DEPENDS
  ${CMAKE_CURRENT_BINARY_DIR}/simcells_help.inc
  ${CMAKE_CURRENT_BINARY_DIR}/simlib_help.inc
)

add_dependencies(yosys generate_cellhelp)

target_include_directories(yosys PUBLIC ${CMAKE_BINARY_DIR})