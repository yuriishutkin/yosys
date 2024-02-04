target_sources(yosys
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/rtlil_backend.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/rtlil_exporter.cc
)
