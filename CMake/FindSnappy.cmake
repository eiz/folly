# - Try to find SNAPPY
#
# The following variables are optionally searched for defaults
#  SNAPPY_ROOT_DIR:            Base directory where all SNAPPY components are found
#
# The following are set after configuration is done: 
#  SNAPPY_FOUND
#  SNAPPY_INCLUDE_DIRS
#  SNAPPY_LIBRARIES
#  SNAPPY_LIBRARY_DIRS

include(FindPackageHandleStandardArgs)

set(SNAPPY_ROOT_DIR "" CACHE PATH "Folder contains SNAPPY") 

find_path(SNAPPY_INCLUDE_DIR snappy.h
    PATHS ${SNAPPY_ROOT_DIR})

if(MSVC)
    if(CMAKE_SIZEOF_VOID_P EQUAL 8)
        find_library(SNAPPY_LIBRARY snappy64
            PATHS ${SNAPPY_ROOT_DIR}/x64/Release)
    else()
        find_library(SNAPPY_LIBRARY snappy
            PATHS ${SNAPPY_ROOT_DIR}/x86/Release)
    endif()

    set(SNAPPY_LIBRARY ${SNAPPY_LIBRARY})
else()
    find_library(SNAPPY_LIBRARY snappy)
endif()

find_package_handle_standard_args(SNAPPY DEFAULT_MSG
    SNAPPY_INCLUDE_DIR SNAPPY_LIBRARY)


if(SNAPPY_FOUND)
    set(SNAPPY_INCLUDE_DIRS ${SNAPPY_INCLUDE_DIR})
    set(SNAPPY_LIBRARIES ${SNAPPY_LIBRARY})
endif()
