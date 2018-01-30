find_package(PythonLibs)
if(${PYTHONLIBS_FOUND})
    message(STATUS "Python version ${PYTHONLIBS_VERSION_STRING} found")
    if(NOT ${PYTHONLIBS_VERSION_STRING} MATCHES "^(2.7)")
        message(FATAL_ERROR "Python version 2.7.x is requred, found - ${PYTHONLIBS_VERSION_STRING}")
    endif()
endif()

