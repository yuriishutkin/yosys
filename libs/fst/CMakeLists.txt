target_sources(yosys
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/fstapi.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/fastlz.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/lz4.cc
)