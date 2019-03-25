# jsbind
# Copyright (c) 2019 Chobolabs Inc.
# http://www.chobolabs.com/
#
# Distributed under the MIT Software License
# See accompanying file LICENSE.txt or copy at
# http://opensource.org/licenses/MIT
#
src_group(jsbind sources
    jsbind.h
    jsbind/common/index_sequence.h
    jsbind/common/function_traits.h
    jsbind/common/wrapped_class.h
    jsbind/common/ptr_cast.h
    jsbind/common/deinitializers.cpp
    jsbind/common/deinitializers.h
    jsbind/funcs.h
    jsbind/value.h
    jsbind/bind.cpp
    jsbind/bind.h
    jsbind/error.h
    jsbind/console.cpp
    jsbind/console.h
    jsbind/exception.cpp
    jsbind/exception.h
)

if((NOT JSBIND_JSC) OR (NOT JSBIND_JSC_NO_TYPED_ARRAYS))
    src_group(jsbind sources
        jsbind/shared_memory_extension.h
    )
endif()

if(JSBIND_JSC)
    src_group(jsc sources
        jsbind/jsc/jsbind.cpp
        jsbind/jsc/value.h
        jsbind/jsc/global.h
        jsbind/jsc/convert.h
        jsbind/jsc/call.h
        jsbind/jsc/bind.h
    )
elseif(JSBIND_V8 OR JSBIND_NODE)
    src_group(v8 sources
        jsbind/v8/jsbind.cpp
        jsbind/v8/value.h
        jsbind/v8/global.h
        jsbind/v8/convert.h
        jsbind/v8/call.h
        jsbind/v8/bind.h
    )
elseif(JSBIND_EMSCRIPTEN)
    src_group(em sources
        jsbind/emscripten/value.h
        jsbind/emscripten/bind.h
        jsbind/emscripten/jsbind.cpp
    )
elseif(JSBIND_CEF)
    src_group(cef sources
        jsbind/cef/jsbind.cpp
        jsbind/cef/value.h
        jsbind/cef/global.h
        jsbind/cef/convert.h
        jsbind/cef/call.h
        jsbind/cef/bind.h
    )
else()
    message(FATAL_ERROR "Unsupported jsbind js engine")
endif()

add_library(jsbind STATIC
    ${sources}
)

target_include_directories(jsbind PUBLIC
    .
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
    message(FATAL_ERROR "Unsupported jsbind js engine")
endif()

target_compile_definitions(jsbind PUBLIC
    ${defs}
)

if(JSBIND_JS_BACKEND_LIBS)
    target_link_libraries(jsbind PUBLIC
        ${JSBIND_JS_BACKEND_LIBS}
    )
endif()