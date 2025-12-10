cmake_minimum_required(VERSION 3.24)
project(xmlwriter)

set(CMAKE_CXX_STANDARD 14)

add_compile_options(
        "-Wall"
        "-Wextra"
        "-Wpedantic"
        "-Werror"
        "-Wfatal-errors"
        "-fexceptions"
        # To allow designated initialisers.
        "-Wno-c99-extensions"
        "-Wno-c++11-extensions"
        "$<$<CONFIG:DEBUG>:-O0;-g3;-ggdb>"
)

function(dump_cmake_variables)
    message(STATUS "==== dump_cmake_variables()")
    get_cmake_property(_variableNames VARIABLES)
    list (SORT _variableNames)
    foreach (_variableName ${_variableNames})
        if (ARGV0)
            unset(MATCHED)
            string(REGEX MATCH ${ARGV0} MATCHED ${_variableName})
            if (NOT MATCHED)
                continue()
            endif()
        endif()
        message(STATUS "${_variableName}=${${_variableName}}")
    endforeach()
    message(STATUS "==== dump_cmake_variables() DONE")
endfunction()

FIND_PACKAGE (Python3 3.13 EXACT REQUIRED COMPONENTS Interpreter Development)
#FIND_PACKAGE(PythonLibs 3.11 EXACT REQUIRED)
#SET(PythonLibs_DIR "/Library/Frameworks/Python.framework/Versions/3.8")
#FIND_PACKAGE(PythonLibs 3.8 REQUIRED PATHS ("/Library/Frameworks/Python.framework/Versions/3.8"))
#FindPythonLibs()
IF (Python3_FOUND)
    INCLUDE_DIRECTORIES("${Python3_INCLUDE_DIRS}")
    #    get_filename_component(PYTHON_LINK_DIRECTORY ${PYTHON_LIBRARY} DIRECTORY)
    # See: https://cmake.org/cmake/help/latest/module/FindPython3.html#module:FindPython3
    message("Python3_VERSION:           ${Python3_VERSION}")
    message("Python3_EXECUTABLE:        ${Python3_EXECUTABLE}")
    message("Python3_INTERPRETER_ID:    ${Python3_INTERPRETER_ID}")
    message("Python3_INCLUDE_DIRS:      ${Python3_INCLUDE_DIRS}")
    message("Python3_STDLIB:            ${Python3_STDLIB}")
    message("Python3_STDARCH:           ${Python3_STDARCH}")
    message("Python3_LINK_OPTIONS:      ${Python3_LINK_OPTIONS}")
    message("Python3_LIBRARIES:         ${Python3_LIBRARIES}")
ELSE ()
    MESSAGE(FATAL_ERROR "Unable to find Python libraries.")
ENDIF ()


include_directories(xmlwriter/cpp)
include_directories(xmlwriter/cpy)
include_directories(xmlwriter/cpy/utils)

add_executable(xmlwriter
        xmlwriter/cpp/base64.cpp
        xmlwriter/cpp/base64.h
        xmlwriter/cpp/main.cpp
        xmlwriter/cpp/XmlWrite.cpp
        xmlwriter/cpp/XmlWrite.h
        xmlwriter/cpy/utils/ConvertPyBytearray.cpp
        xmlwriter/cpy/utils/ConvertPyBytearray.h
        xmlwriter/cpy/utils/ConvertPyBytes.cpp
        xmlwriter/cpy/utils/ConvertPyBytes.h
        xmlwriter/cpy/utils/ConvertPyDict.h
        xmlwriter/cpy/utils/ConvertPyList.h
        xmlwriter/cpy/utils/ConvertPySet.h
        xmlwriter/cpy/utils/ConvertPyStr.cpp
        xmlwriter/cpy/utils/ConvertPyStr.h
        xmlwriter/cpy/utils/cpython_asserts.cpp
        xmlwriter/cpy/utils/cpython_asserts.h
        xmlwriter/cpy/utils/CPythonUtils.h
        xmlwriter/cpy/utils/DefaultArguments.h
        xmlwriter/cpy/utils/PyObjectWrapper.h
        xmlwriter/cpy/utils/RefWrapper.h
        xmlwriter/cpy/utils/TestCPythonUtils.cpp
        xmlwriter/cpy/utils/TestCPythonUtils.h
        xmlwriter/cpy/cXmlWrite.cpp
#        xmlwriter/cpy/pbXmlWrite.cpp
        xmlwriter/cpy/XmlWrite_docs.cpp
        xmlwriter/cpy/XmlWrite_docs.h)

link_directories(${Python3_LIBRARIES})

#target_link_libraries(${PROJECT_NAME} ${PYTHON_LIBRARY})
target_link_libraries(${PROJECT_NAME} ${Python3_LIBRARIES})

MESSAGE(STATUS "Build type: " ${CMAKE_BUILD_TYPE})
MESSAGE(STATUS "Library Type: " ${LIB_TYPE})
MESSAGE(STATUS "Compiler flags:" ${CMAKE_CXX_COMPILE_FLAGS})
MESSAGE(STATUS "Compiler cxx debug flags:" ${CMAKE_CXX_FLAGS_DEBUG})
MESSAGE(STATUS "Compiler cxx release flags:" ${CMAKE_CXX_FLAGS_RELEASE})
MESSAGE(STATUS "Compiler cxx min size flags:" ${CMAKE_CXX_FLAGS_MINSIZEREL})
MESSAGE(STATUS "Compiler cxx flags:" ${CMAKE_CXX_FLAGS})

dump_cmake_variables()
