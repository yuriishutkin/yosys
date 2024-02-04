
if(SMALL)
target_sources(yosys
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/design.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/select.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/show.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/stat.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/plugin.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/cover.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/clean_zerowidth.cc
)
else()
  file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)
  target_sources(yosys PRIVATE ${SOURCES})
endif()
