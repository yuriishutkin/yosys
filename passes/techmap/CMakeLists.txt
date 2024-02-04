
if(SMALL)
target_sources(yosys
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/flatten.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/techmap.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/simplemap.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/dfflibmap.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/maccmap.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/booth.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/libparse.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/bmuxmap.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/demuxmap.cc
)
else()
  file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)
  target_sources(yosys PRIVATE ${SOURCES})
endif()
