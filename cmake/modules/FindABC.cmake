include(FindPackageHandleStandardArgs)

set(ABC_FOUND false)

find_library(ABC_LIBRARY NAMES abc)
find_program(ABC_EXECUTABLE NAMES abc)

find_package_handle_standard_args(ABC
  REQUIRED_VARS
  ABC_LIBRARY
  ABC_EXECUTABLE
)

if(ABC_FOUND)
  mark_as_advanced(ABC_LIBRARY)
  mark_as_advanced(ABC_EXECUTABLE)
  
  add_library(ABC::ABC UNKNOWN IMPORTED)
  set_property(TARGET ABC::ABC
    PROPERTY
    IMPORTED_LOCATION ${ABC_LIBRARY}
  )
endif()