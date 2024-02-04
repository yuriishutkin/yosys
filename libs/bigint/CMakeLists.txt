target_sources(yosys
  PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/BigInteger.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/BigIntegerAlgorithms.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/BigIntegerUtils.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/BigUnsigned.cc
  ${CMAKE_CURRENT_SOURCE_DIR}/BigUnsignedInABase.cc
)