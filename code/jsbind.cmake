# jsbind
# Copyright (c) 2019 Chobolabs Inc.
# http://www.chobolabs.com/
#
# Distributed under the MIT Software License
# See accompanying file LICENSE.txt or copy at
# http://opensource.org/licenses/MIT
#
if(NOT COMMAND src_group)
    macro(src_group GROUP_NAME SRC_LIST)
        source_group(${GROUP_NAME} FILES ${ARGN})
        foreach(filename ${ARGN})
            list(APPEND ${SRC_LIST} ${filename})
        endforeach()
    endmacro(src_group)
endif()

set(sources)
set(code ${CMAKE_CURRENT_LIST_DIR})
src_group(jsbind sources
    ${code}/jsbind.hpp
    ${code}/jsbind/common/index_sequence.hpp
    ${code}/jsbind/common/function_traits.hpp
    ${code}/jsbind/common/wrapped_class.hpp
    ${code}/jsbind/common/ptr_cast.hpp
    ${code}/jsbind/common/deinitializers.cpp
    ${code}/jsbind/common/deinitializers.hpp
    ${code}/jsbind/funcs.hpp
    ${code}/jsbind/value.hpp
    ${code}/jsbind/bind.cpp
    ${code}/jsbind/bind.hpp
    ${code}/jsbind/error.hpp
    ${code}/jsbind/console.cpp
    ${code}/jsbind/console.hpp
    ${code}/jsbind/exception.cpp
    ${code}/jsbind/exception.hpp
)

if((NOT JSBIND_JSC) OR (NOT JSBIND_JSC_NO_TYPED_ARRAYS))
    src_group(jsbind sources
        ${code}/jsbind/shared_memory_extension.hpp
    )
endif()

if(JSBIND_JSC)
    src_group(jsc sources
        ${code}/jsbind/jsc/jsbind.cpp
        ${code}/jsbind/jsc/value.hpp
        ${code}/jsbind/jsc/global.hpp
        ${code}/jsbind/jsc/convert.hpp
        ${code}/jsbind/jsc/call.hpp
        ${code}/jsbind/jsc/bind.hpp
    )
elseif(JSBIND_V8 OR JSBIND_NODE)
    src_group(v8 sources
        ${code}/jsbind/v8/jsbind.cpp
        ${code}/jsbind/v8/value.hpp
        ${code}/jsbind/v8/global.hpp
        ${code}/jsbind/v8/convert.hpp
        ${code}/jsbind/v8/call.hpp
        ${code}/jsbind/v8/bind.hpp
    )
elseif(JSBIND_EMSCRIPTEN)
    src_group(em sources
        ${code}/jsbind/emscripten/value.hpp
        ${code}/jsbind/emscripten/bind.hpp
        ${code}/jsbind/emscripten/jsbind.cpp
    )
elseif(JSBIND_CEF)
    src_group(cef sources
        ${code}/jsbind/cef/jsbind.cpp
        ${code}/jsbind/cef/value.hpp
        ${code}/jsbind/cef/global.hpp
        ${code}/jsbind/cef/convert.hpp
        ${code}/jsbind/cef/call.hpp
        ${code}/jsbind/cef/bind.hpp
    )
else()
    message(FATAL_ERROR "JSBind: Unsupported jsbind js engine")
endif()

if(NOT JSBIND_TARGET_NAME)
    message(FATAL_ERROR "JSBind: You must set JSBIND_TARGET_NAME when you're including jsbind.cmake")
endif()

add_library(${JSBIND_TARGET_NAME} STATIC
    ${sources}
)

target_include_directories(${JSBIND_TARGET_NAME} PUBLIC
    ${code}
)

set(defs)

if(JSBIND_DEBUGGING)
    set(defs ${defs} -DJSBIND_DEBUGGING)
endif()

if(JSBIND_NODE)
    set(defs ${defs} -DJSBIND_V8 -DJSBIND_NODE -DBUILDING_NODE_EXTENSION)
elseif(JSBIND_V8)
    set(defs ${defs} -DJSBIND_V8)
elseif(JSBIND_JSC)
    set(defs ${defs} -DJSBIND_JSC)
    if(NOT JSBIND_JSC_NO_TYPED_ARRAYS)
        set(defs ${defs} -D_JSC_TYPED_ARRAYS)
    endif()
elseif(JSBIND_EMSCRIPTEN)
    set(defs ${defs} -DJSBIND_EMSCRIPTEN)
elseif(JSBIND_CEF)
    set(defs ${defs} -DJSBIND_CEF)
else()
    message(FATAL_ERROR "JSBind: Unsupported jsbind js engine")
endif()

target_compile_definitions(${JSBIND_TARGET_NAME} PUBLIC
    ${defs}
)

if(JSBIND_JS_BACKEND_LIBS)
    target_link_libraries(${JSBIND_TARGET_NAME} PUBLIC
        ${JSBIND_JS_BACKEND_LIBS}
    )
endif()