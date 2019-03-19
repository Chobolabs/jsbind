
cmake_minimum_required(VERSION 3.1)

project(jsbind)

option(JSBIND_ENABLE_TESTING "JSBind: Enable testing" OFF)
option(JSBIND_BUILD_EXAMPLES "JSBind: Build examples" OFF)
option(JSBIND_DEBUGGING "JSBind: Enable internal debugging features" OFF)

option(JSBIND_V8 "JSBind: Use v8 as underlying JS engine" OFF)

option(JSBIND_NODE "JSBind: Use Node as underlying JS backend" OFF)

option(JSBIND_JSC "JSBind: Use JSC as underlying JS engine" OFF)
option(JSBIND_JSC_TYPED_ARRAYS "JSBind: Use typed array from modern JSC" OFF)
option(JSBIND_JSC_PATCH_TYPED_ARRAYS "JSBind: Use typed array patch from phoboslabs for JSC" OFF)

option(JSBIND_EMSCRIPTEN "JSBind: Use emscripten as underlying JS backend" OFF)

option(JSBIND_CEF "JSBind: Use CEF as underlying JS backend" OFF)

set(CODE ${CMAKE_CURRENT_SOURCE_DIR}/code)

set(sources)

src_group(jsbind sources
    ${CODE}/jsbind.h
    ${CODE}/jsbind/common/index_sequence.h
    ${CODE}/jsbind/common/function_traits.h
    ${CODE}/jsbind/common/wrapped_class.h
    ${CODE}/jsbind/common/ptr_cast.h
    ${CODE}/jsbind/common/deinitializers.cpp
    ${CODE}/jsbind/common/deinitializers.h
    ${CODE}/jsbind/funcs.h
    ${CODE}/jsbind/value.h
    ${CODE}/jsbind/bind.cpp
    ${CODE}/jsbind/bind.h
    ${CODE}/jsbind/error.h
    ${CODE}/jsbind/console.cpp
    ${CODE}/jsbind/console.h
    ${CODE}/jsbind/exception.cpp
    ${CODE}/jsbind/exception.h
)

if((NOT JSBIND_JSC) OR JSBIND_JSC_TYPED_ARRAYS OR JSBIND_JSC_PATCH_TYPED_ARRAYS)
    src_group(jsbind sources
        ${CODE}/jsbind/shared_memory_extension.h
    )
endif()

if(JSBIND_JSC)
    src_group(jsc sources
        ${CODE}/jsbind/jsc/jsbind.cpp
        ${CODE}/jsbind/jsc/value.h
        ${CODE}/jsbind/jsc/global.h
        ${CODE}/jsbind/jsc/convert.h
        ${CODE}/jsbind/jsc/call.h
        ${CODE}/jsbind/jsc/bind.h
    )
elseif(JSBIND_V8 OR JSBIND_NODE)
    src_group(v8 sources
        ${CODE}/jsbind/v8/jsbind.cpp
        ${CODE}/jsbind/v8/value.h
        ${CODE}/jsbind/v8/global.h
        ${CODE}/jsbind/v8/convert.h
        ${CODE}/jsbind/v8/call.h
        ${CODE}/jsbind/v8/bind.h
    )
elseif(JSBIND_EMSCRIPTEN)
    src_group(em sources
        ${CODE}/jsbind/emscripten/value.h
        ${CODE}/jsbind/emscripten/bind.h
        ${CODE}/jsbind/emscripten/jsbind.cpp
    )
elseif(JSBIND_CEF)
    src_group(cef sources
        ${CODE}/jsbind/cef/jsbind.cpp
        ${CODE}/jsbind/cef/value.h
        ${CODE}/jsbind/cef/global.h
        ${CODE}/jsbind/cef/convert.h
        ${CODE}/jsbind/cef/call.h
        ${CODE}/jsbind/cef/bind.h
    )
endif()

add_library(jsbind STATIC
    ${sources}
)

target_include_directories(jsbind PUBLIC
    ${CODE}
)

set(defs)
set(js_engine_libs)

if(JSBIND_DEBUGGING)
    set(defs ${defs} -DJSBIND_DEBUGGING)
endif()

if(JSBIND_NODE)
    if (NOT DEFINED node_version)
        execute_process(COMMAND node --version OUTPUT_VARIABLE node_version)
        string(SUBSTRING ${node_version} 1 -1 node_version)
        string(STRIP ${node_version} node_version)
    endif()

    set(defs ${defs} -DJSBIND_V8 -DJSBIND_NODE -DBUILDING_NODE_EXTENSION)
elseif(JSBIND_V8)
    set(defs ${defs} -DJSBIND_V8)
elseif(JSBIND_JSC)
    set(defs ${defs} -DJSBIND_JSC)
    if(JSBIND_JSC_TYPED_ARRAYS)
        set(defs ${defs} -D_JSC_TYPED_ARRAYS)
    endif()
elseif(JSBIND_EMSCRIPTEN)
    set(defs ${defs} -DJSBIND_EMSCRIPTEN)
elseif(JSBIND_CEF)
    set(defs ${defs} -DJSBIND_CEF)
    set(js_engine_libs cefdll_wrapper)
else()
    message(FATAL_ERROR "Unsupported jsbind js engine")
endif()

target_compile_definitions(jsbind PUBLIC
    ${defs}
)

target_link_libraries(jsbind PUBLIC
    ${js_engine_libs}
)

macro(add_jsbind_exe target)
    if(JSBIND_NODE)
        add_library(${target} SHARED
            "${ARGN}"
        )

        target_compile_definitions(${target} PRIVATE
            -DNODE_GYP_MODULE_NAME=${target}
            -DBUILDING_V8_SHARED=1
            -DBUILDING_UV_SHARED=1
            -DBUILDING_NODE_EXTENSION
        )

        if(chobo_target_macos)
            set_target_properties(${target} PROPERTIES LINK_FLAGS "-undefined dynamic_lookup \
                -Wl,-no_pie \
                -Wl,-search_paths_first"
            )
        endif()

        set_target_properties(${target} PROPERTIES PREFIX "" SUFFIX ".node")
    elseif(JSBIND_CEF)
        add_library(${target} STATIC
            "${ARGN}"
        )
    else()
        add_executable(${target}
            "${ARGN}"
        )

        if(chobo_target_emscripten)
            set_target_properties(${target} PROPERTIES
                #LINK_FLAGS "--bind --preload-file ${CMAKE_CURRENT_SOURCE_DIR}/script@/")
                LINK_FLAGS "--bind")
        elseif(chobo_target_ios)
            set_target_properties(${target} PROPERTIES
                MACOSX_BUNDLE_BUNDLE_NAME "${APP_PRETTY_NAME}"
                MACOSX_BUNDLE_INFO_PLIST "${chobo_root}/build/platform/ios/proj/Info.plist.in"
                XCODE_ATTRIBUTE_INSTALL_PATH "$(LOCAL_APPS_DIR)"
                XCODE_ATTRIBUTE_SKIP_INSTALL "No"
                XCODE_ATTRIBUTE_CODE_SIGN_IDENTITY "iPhone Developer")
        endif()
    endif()

    target_link_libraries(${target}
        jsbind
    )
endmacro()

if(JSBIND_BUILD_EXAMPLES)
    add_jsbind_exe(jsbind-example ${CMAKE_CURRENT_SOURCE_DIR}/example/main.cpp)
endif()

# tests
if(JSBIND_ENABLE_TESTING)
    if(NOT JSBIND_CEF)
        enable_testing()
    endif()

    set(test_sources
        ${CMAKE_CURRENT_SOURCE_DIR}/test/test.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/person.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/person.h
        ${CMAKE_CURRENT_SOURCE_DIR}/test/testclass.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/testclass.h
        ${CMAKE_CURRENT_SOURCE_DIR}/test/pods.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/test/pods.h
    )

    if((NOT JSBIND_JSC) OR JSBIND_JSC_TYPED_ARRAYS OR JSBIND_JSC_PATCH_TYPED_ARRAYS)
        set(test_sources ${test_sources} ${CMAKE_CURRENT_SOURCE_DIR}/test/shared_memory_extension_test.cpp)
        # add_jsbind_exe(shared-mem-test ${CMAKE_CURRENT_SOURCE_DIR}/test/shared_memory_extension.cpp)
    endif()

    add_jsbind_exe(jsbind-test ${test_sources})

    macro(add_jsbind_test name)
        if(chobo_target_emscripten OR JSBIND_NODE)
            add_test(NAME ${name} COMMAND node ${name}.js)
        elseif(JSBIND_CEF)
            target_compile_definitions(${name} INTERFACE -DJSBIND_TESTING=1)
        else()
            add_test(${name} ${name})
        endif()
    endmacro()

    add_jsbind_test(jsbind-test)
endif()
