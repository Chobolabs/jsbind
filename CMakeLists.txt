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

mark_as_advanced(JSBIND_ENABLE_TESTING JSBIND_BUILD_EXAMPLES JSBIND_DEBUGGING)

if(NOT JSBIND_TARGET_NAME)
    message(STATUS "JSBind: JSBIND_TARGET_NAME is not set. Defaulting to `jsbind`")
    set(JSBIND_TARGET_NAME jsbind)
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
            message(FATAL_ERROR "JSBind: Set the appropriate emscripten toolchain file")
        endif()
        # nothing more to do all we need comes from the outside
    elseif(JSBIND_NODE)
        if (NOT DEFINED JSBIND_NODE_VERSION)
            execute_process(COMMAND node --version OUTPUT_VARIABLE JSBIND_NODE_VERSION)
            string(SUBSTRING ${JSBIND_NODE_VERSION} 1 -1 JSBIND_NODE_VERSION)
            string(STRIP ${JSBIND_NODE_VERSION} JSBIND_NODE_VERSION)
        endif()

        add_library(jsbind-node-target INTERFACE)

        if(MSVC)
            file(TO_CMAKE_PATH "$ENV{HOMEDRIVE}$ENV{HOMEPATH}" usrHome)
            target_include_directories(jsbind-node-target INTERFACE
                ${usrHome}/.node-gyp/${JSBIND_NODE_VERSION}/include/node
            )
            if(CMAKE_SIZEOF_VOID_P EQUAL 8)
                target_link_libraries(jsbind-node-target INTERFACE
                    ${usrHome}/.node-gyp/${JSBIND_NODE_VERSION}/x64/node.lib
                )
            else()
                target_link_libraries(jsbind-node-target INTERFACE
                    ${usrHome}/.node-gyp/${JSBIND_NODE_VERSION}/ia32/node.lib
                )
            endif()
            target_compile_options(jsbind-node-target INTERFACE
                /wd4251
                /wd4275
            )
        elseif(APPLE)
        else() # assuming linux
            target_include_directories(jsbind-node-target INTERFACE
                ~/.node-gyp/${JSBIND_NODE_VERSION}/include/node
            )
            target_compile_options(jsbind-node-target INTERFACE
                -fPIC # possibly add this to jsbind itself
                      # after all we're always a shared library in a node context
            )
        endif()

        set(JSBIND_JS_BACKEND_LIBS jsbind-node-target)
    elseif(JSBIND_CEF)
        if(NOT JSBIND_CEF_CEF_CMAKE_DIR) # could be external
            set(JSBIND_CEF_CEF_CMAKE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/cef-cmake)
        endif()

        set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
        set(CEF_USE_SANDBOX OFF CACHE BOOL "Force turning off of CEF sandbox")
        include(${JSBIND_CEF_CEF_CMAKE_DIR}/cmake/cef_cmake.cmake)
        add_subdirectory(${JSBIND_CEF_CEF_CMAKE_DIR} cef-cmake)
        set(JSBIND_JS_BACKEND_LIBS cefdll_wrapper)
    elseif(JSBIND_JSC)
        if(NOT JSBIND_JSC_JSC_STANDALONE_DIR)
            set(JSBIND_JSC_JSC_STANDALONE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/jsc-standalone)
        endif()
        add_subdirectory(${JSBIND_JSC_JSC_STANDALONE_DIR} jsc-standalone)
        set(JSBIND_JS_BACKEND_LIBS javascriptcore)
    elseif(JSBIND_V8)
        if(MSVC)
            if((${MSVC_VERSION} LESS 1910) OR (${MSVC_VERSION} GREATER 1919) OR (CMAKE_SIZEOF_VOID_P EQUAL 4))
                message(FATAL_ERROR "JSBind: Only Visual Studio 15 2017 Win64 supported for tests")
            endif()
            add_library(jsbind-v8-target INTERFACE)
            target_include_directories(jsbind-v8-target INTERFACE
                ${CMAKE_CURRENT_SOURCE_DIR}/third_party/v8-v141-x64.7.4.288.11/include
            )
            target_link_libraries(jsbind-v8-target INTERFACE
                debug ${CMAKE_CURRENT_SOURCE_DIR}/third_party/v8-v141-x64.7.4.288.11/lib/Debug/v8.dll.lib
                debug ${CMAKE_CURRENT_SOURCE_DIR}/third_party/v8-v141-x64.7.4.288.11/lib/Debug/v8_libbase.dll.lib
                debug ${CMAKE_CURRENT_SOURCE_DIR}/third_party/v8-v141-x64.7.4.288.11/lib/Debug/v8_libplatform.dll.lib
                optimized ${CMAKE_CURRENT_SOURCE_DIR}/third_party/v8-v141-x64.7.4.288.11/lib/Release/v8.dll.lib
                optimized ${CMAKE_CURRENT_SOURCE_DIR}/third_party/v8-v141-x64.7.4.288.11/lib/Release/v8_libbase.dll.lib
                optimized ${CMAKE_CURRENT_SOURCE_DIR}/third_party/v8-v141-x64.7.4.288.11/lib/Release/v8_libplatform.dll.lib
            )
        else()
            message(FATAL_ERROR "JSBind: Unsupported v8 config for tests")
        endif()

        set(JSBIND_JS_BACKEND_LIBS jsbind-v8-target)
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
