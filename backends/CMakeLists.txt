
if(SMALL)
add_subdirectory(rtlil)
add_subdirectory(verilog)
else()
include_all_subdirs()
endif()

