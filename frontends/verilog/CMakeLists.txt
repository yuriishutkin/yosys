target_sources(yosys
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/preproc.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/verilog_frontend.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/const2ast.cc
)
