# jsbind
# Copyright (c) 2019 Chobolabs Inc.
# http://www.chobolabs.com/
#
# Distributed under the MIT Software License
# See accompanying file LICENSE.txt or copy at
# http://opensource.org/licenses/MIT
#
cmake_minimum_required(VERSION 3.1)

project(jsbind)

option(JSBIND_ENABLE_TESTING "JSBind: Enable testing" OFF)
option(JSBIND_CREATE_TEST_LIB "JBind: Add test libary for external testing" OFF)
option(JSBIND_BUILD_EXAMPLES "JSBind: Build examples" OFF)
option(JSBIND_DEBUGGING "JSBind: Enable internal debugging features" OFF)

option(JSBIND_V8 "JSBind: Use v8 as underlying JS engine" OFF)

option(JSBIND_NODE "JSBind: Use Node as underlying JS backend" OFF)

option(JSBIND_JSC "JSBind: Use JSC as underlying JS engine" OFF)

option(JSBIND_EMSCRIPTEN "JSBind: Use emscripten as underlying JS backend" OFF)

option(JSBIND_CEF "JSBind: Use CEF as underlying JS backend" OFF)

mark_as_advanced(JSBIND_ENABLE_TESTING JSBIND_BUILD_EXAMPLES)

set(sources)

if(NOT COMMAND src_group)
    macro(src_group GROUP_NAME SRC_LIST)
        source_group(${GROUP_NAME} FILES ${ARGN})
        foreach(filename ${ARGN})
            list(APPEND ${SRC_LIST} ${filename})
        endforeach()
    endmacro(src_group)
endif()

################################################################################
if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    # Danger zone!
    # The common case of using jsbind wouldn't enter this block.
    # The following code relies on a development only setup which is not
    # configured by this cmake file.
    # Read the docs about building jsbind standalone or running tests and
    # examples to learn how to set up the required configuration.

    if(JSBIND_ENABLE_TESTING)
        enable_testing()
        add_library(doctest INTERFACE)
        target_include_directories(doctest INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/test)
        set(JBIND_TESTLIB_DOCTEST_TARGET doctest)
    endif()

    set(CMAKE_CXX_STANDARD 11)

    if(JSBIND_EMSCRIPTEN)
        if(NOT EMSCRIPTEN)
            message(FATAL_ERROR "Set the appropriate emscripten toolchain file")
        endif()
        # nothing more to do all we need comes from the outside
    else()
        message(FATAL_ERROR "JSBind: Unsupported jsbind js engine")
    endif()

endif()
################################################################################

# adding the jsbind library
add_subdirectory(code)

if(JSBIND_ENABLE_TESTING OR JSBIND_CREATE_TEST_LIB)
    add_subdirectory(testlib)
endif()

if(JSBIND_ENABLE_TESTING)
    add_subdirectory(test)
endif()

