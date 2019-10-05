project(sweetPy CXX)
cmake_minimum_required(VERSION 3.0)

set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_COLOR_MAKEFILE ON)
set (CMAKE_CXX_EXTENSIONS OFF)

if(NOT WIN32)
    string(ASCII 27 Esc)
    set(ColourReset "${Esc}[m")
    set(Red "${Esc}[31m")
    set(Green "${Esc}[32m")
endif()

if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "Default build type 'Debug'")
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE )
else()
    message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
endif()

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(CMAKE_DEBUG_POSTFIX d)
endif()

set(PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(PROJECT_BUILD_DIR ${CMAKE_BINARY_DIR})
if(NOT OUTPUT_DIR)
    set(OUTPUT_DIR ${PROJECT_BUILD_DIR}/out)
endif()

if(NOT sweetPy_3RD_PARTY_DIR)
    set(sweetPy_3RD_PARTY_DIR ${PROJECT_BUILD_DIR}/Third_Party/)
endif()
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
list(APPEND CMAKE_PREFIX_PATH ${sweetPy_3RD_PARTY_DIR})

option(WITH_TESTS "Test support" OFF)
option(WITH_EXAMPLES "Examples support" OFF)
option(sweetPy_3RD_PARTY_INSTALL_STEP "3rd parties installation step" OFF)
option(sweetPy_COMPILATION_STEP "Compilation step" OFF)

set(IS_PY_DEBUG OFF)
include(Utility)
IS_PYTHON_DEBUG()

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
find_package(Python REQUIRED)
find_package(Core)
find_package(FlatBuffers)

SET(DEPENDECIES "")
SET(TEST_DEPENDECIES "")
if(WITH_TESTS)
    find_package(GoogleTest)
endif()

include(installThirdParty)

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-fPIC -DSPDLOG_FOUND=${SPDLOG_FOUND} -DTHRIFT_FOUND=${THRIFT_FOUND} -Wall -Werror -pedantic -Wno-sign-compare -Wno-unused-variable -Wno-unused-but-set-variable")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BUILD_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY  ${PROJECT_BUILD_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${PROJECT_BUILD_DIR}/bin)
set(CMAKE_BINARY_DIR ${PROJECT_BUILD_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BUILD_DIR}/bin)
#Pre build step

include_directories(${PROJECT_DIR}/include ${sweetPy_3RD_PARTY_DIR}/include ${PYTHON_INCLUDE_DIRS})
add_library(sweetPy SHARED src/Detail/CPythonType.cpp src/Detail/MetaClass.cpp src/Core/Lock.cpp src/Types/Container.cpp src/Utility/Serialize.cpp src/Utility/SerializeTypes.cpp)
target_link_libraries(sweetPy ${sweetPy_3RD_PARTY_DIR}/lib/libCore${CMAKE_DEBUG_POSTFIX}.so)
if(IS_PY_DEBUG)
    message(STATUS "python debug macros are set - Py_TRACE_REF, Py_DEBUG, LLTRACE, Py_REF_DEBUG")
    add_definitions(-DPy_TRACE_REF -DPy_DEBUG -DLLTRACE -DPy_REF_DEBUG)
endif()
if(DEPENDECIES)
    add_dependencies(sweetPy ${DEPENDECIES})
endif()

if(WITH_EXAMPLES)
    add_subdirectory(${PROJECT_DIR}/example)
endif()
if(WITH_TESTS)
    add_subdirectory(${PROJECT_DIR}/Tests)
endif()

install(TARGETS sweetPy
        LIBRARY DESTINATION ${OUTPUT_DIR}/lib
        )
install(DIRECTORY ${PROJECT_DIR}/include/ DESTINATION ${OUTPUT_DIR}/include/sweetPy/)
