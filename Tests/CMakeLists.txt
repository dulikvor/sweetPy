project(sweetPyTests CXX)
cmake_minimum_required(VERSION 3.0)

set (CMAKE_CXX_STANDARD 17)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_COLOR_MAKEFILE ON)
set (CMAKE_CXX_EXTENSIONS OFF)

if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "Default build type 'Debug'")
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "" FORCE )
else()
    message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
endif()

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
    set(CMAKE_DEBUG_POSTFIX)
    set(CORE_DEBUG_POSTFIX d)
endif()

set(PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_BUILD_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY  ${PROJECT_BUILD_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${PROJECT_BUILD_DIR}/bin)
set(CMAKE_BINARY_DIR ${PROJECT_BUILD_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BUILD_DIR}/bin)

link_directories(${sweetPy_3RD_PARTY_DIR}/lib ${PROJECT_DIR}/../bin)

add_library(CPythonClassTestModule SHARED CPythonClassTestModule.cpp)
target_link_libraries(CPythonClassTestModule sweetPy ${sweetPy_3RD_PARTY_DIR}/lib/libCore${CORE_DEBUG_POSTFIX}.so pthread )
set_target_properties(CPythonClassTestModule PROPERTIES PREFIX "")
target_include_directories(CPythonClassTestModule PRIVATE . .. ../include ${sweetPy_3RD_PARTY_DIR}/include ${PYTHON_INCLUDE_DIRS})

add_executable(sweetPyTests Tests.cpp CPythonClassTestModule.h)
target_include_directories(sweetPyTests PRIVATE . .. ../include ${sweetPy_3RD_PARTY_DIR}/include ${PYTHON_INCLUDE_DIRS})
if(TEST_DEPENDECIES)
    add_dependencies(sweetPyTests ${TEST_DEPENDECIES})
endif()
set(TO_LINK_LIBS CPythonClassTestModule gtest gtest_main ${PYTHON_LIBRARIES} util dl ${sweetPy_3RD_PARTY_DIR}/lib/libCore${CORE_DEBUG_POSTFIX}.so pthread)
target_link_libraries(sweetPyTests ${TO_LINK_LIBS})
