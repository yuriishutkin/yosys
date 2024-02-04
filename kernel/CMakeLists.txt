file(WRITE
  ${CMAKE_CURRENT_BINARY_DIR}/yosys_ver.cc
  "namespace Yosys { extern const char *yosys_version_str; const char *yosys_version_str=\"Yosys CMake Version\"; }"
)

target_sources(yosys
  PRIVATE
  ${CMAKE_CURRENT_BINARY_DIR}/yosys_ver.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/binding.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/calc.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/cellaigs.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/celledges.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/ff.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/ffmerge.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/fmt.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/fstdata.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/json.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/log.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/mem.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/qcsat.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/register.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/rtlil.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/satgen.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/yosys.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/yw.cc
)