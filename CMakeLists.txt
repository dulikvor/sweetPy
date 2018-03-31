project(pyCppConn CXX)
cmake_minimum_required(VERSION 3.0)

set (CMAKE_CXX_STANDARD 14)
set (CMAKE_CXX_STANDARD_REQUIRED ON)
set (CMAKE_COLOR_MAKEFILE ON)
set (CMAKE_CXX_EXTENSIONS OFF)

if (NOT CMAKE_BUILD_TYPE)
    message(STATUS "Default build type 'Debug'")
    set(CMAKE_BUILD_TYPE DEBUG CACHE STRING "" FORCE )
else()
    string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE)
    message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
endif()

set(PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
find_package(Python REQUIRED)

if(pyCppConn_Test_Support)
    set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
    list(APPEND CMAKE_PREFIX_PATH ${CURRENT_PROJECT_3RD_LOC})
    find_package(GoogleTest)
    include(installThirdParty)
endif()

include_directories( ${PROJECT_DIR} ${PROJECT_DIR}/src ${PYTHON_INCLUDE_DIRS})

set(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS} "-fPIC -DSPDLOG_FOUND=${SPDLOG_FOUND} -DTHRIFT_FOUND=${THRIFT_FOUND}")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY  ${PROJECT_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY  ${PROJECT_DIR}/bin)
set(CMAKE_BINARY_DIR ${PROJECT_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_DIR}/bin)
#Pre build step

set(CORE_COMPILE True)
set(CORE_SPDLOG_SUPPORT OFF)
add_subdirectory(${PROJECT_DIR}/Core)
add_subdirectory(${PROJECT_DIR}/example)
if(pyCppConn_Test_Support)
    add_subdirectory(${PROJECT_DIR}/Tests)
endif()

add_definitions(-DA_DLL)
add_library(pyCppConn SHARED src/CPythonModule.h src/CPythonModule.cpp src/CPythonModule.h src/Common.h src/Exception.cpp src/Exception.h src/CPythonClass.h src/Lock.h src/Deleter.h src/CPythonFunction.h src/CPythonMember.h src/CPyModuleContainer.cpp src/CPyModuleContainer.h src/CPythonMetaClass.cpp src/CPythonMetaClass.h src/CPythonConstructor.h src/CPythonObject.h Tests/PythonEmbedder.h src/CPythonObject.cpp)
