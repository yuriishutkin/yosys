
if(SMALL)
add_subdirectory(ast)
add_subdirectory(blif)
add_subdirectory(rtlil)
add_subdirectory(verilog)
else()
include_all_subdirs()
endif()
