project(pyCppConn CXX)
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
    set(CMAKE_BUILD_TYPE DEBUG CACHE STRING "" FORCE )
else()
    string(TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPE)
    message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
endif()

set(PROJECT_DIR ${CMAKE_CURRENT_SOURCE_DIR})
if(NOT pyCppConn_3RD_PARTY_DIR)
    set(pyCppConn_3RD_PARTY_DIR ${PROJECT_DIR}/Third_Party/)
endif()
set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
list(APPEND CMAKE_PREFIX_PATH ${pyCppConn_3RD_PARTY_DIR})

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake)
find_package(Python REQUIRED)
find_package(Core)

option(pyCppConn_Test_Support "Test support" OFF)
if(pyCppConn_Test_Support)
    find_package(GoogleTest)
endif()

if(3RD_PARTY_INSTALL_STEP)
    include(installThirdParty)
endif()


if(COMPILATION_STEP)
    include_directories( ${PROJECT_DIR} ${PROJECT_DIR}/src ${pyCppConn_3RD_PARTY_DIR}/include ${PYTHON_INCLUDE_DIRS})

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
    add_subdirectory(${PROJECT_DIR}/example)
    if(pyCppConn_Test_Support)
        add_subdirectory(${PROJECT_DIR}/Tests)
    endif()

    add_definitions(-DA_DLL)
    add_library(pyCppConn SHARED src/CPythonModule.h src/CPythonModule.cpp src/CPythonModule.h src/Common.h src/Exception.cpp src/Exception.h src/CPythonClass.h src/Lock.h src/Deleter.h src/CPythonFunction.h src/CPythonMember.h src/CPyModuleContainer.cpp src/CPyModuleContainer.h src/CPythonMetaClass.cpp src/CPythonMetaClass.h src/CPythonConstructor.h src/CPythonObject.h Tests/PythonEmbedder.h src/CPythonRefObject.cpp src/CPythonEnum.h src/CPythonEnumValue.h src/CPythonEnum.cpp src/TypeState.h src/ICPythonFunction.h src/InitModule.h)
    target_link_libraries(pyCppConn ${pyCppConn_3RD_PARTY_DIR}/lib/libCore.so)
endif()
