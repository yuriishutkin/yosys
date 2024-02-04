if(SMALL)
target_sources(yosys
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/opt.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/opt_merge.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/opt_mem.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/opt_mem_feedback.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/opt_mem_priority.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/opt_mem_widen.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/opt_muxtree.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/opt_reduce.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/opt_dff.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/opt_share.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/opt_clean.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/opt_expr.cc
)
else()
  file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cc)
  target_sources(yosys PRIVATE ${SOURCES})
endif()