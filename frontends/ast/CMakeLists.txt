target_sources(yosys
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/ast.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/simplify.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/genrtlil.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/dpicall.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/ast_binding.cc
)
