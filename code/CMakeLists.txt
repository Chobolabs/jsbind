# jsbind
# Copyright (c) 2019 Chobolabs Inc.
# http://www.chobolabs.com/
#
# Distributed under the MIT Software License
# See accompanying file LICENSE.txt or copy at
# http://opensource.org/licenses/MIT
#
src_group(jsbind sources
    jsbind.hpp
    jsbind/common/index_sequence.hpp
    jsbind/common/function_traits.hpp
    jsbind/common/wrapped_class.hpp
    jsbind/common/ptr_cast.hpp
    jsbind/common/deinitializers.cpp
    jsbind/common/deinitializers.hpp
    jsbind/funcs.hpp
    jsbind/value.hpp
    jsbind/bind.cpp
    jsbind/bind.hpp
    jsbind/error.hpp
    jsbind/console.cpp
    jsbind/console.hpp
    jsbind/exception.cpp
    jsbind/exception.hpp
)

if((NOT JSBIND_JSC) OR (NOT JSBIND_JSC_NO_TYPED_ARRAYS))
    src_group(jsbind sources
        jsbind/shared_memory_extension.hpp
    )
endif()

if(JSBIND_JSC)
    src_group(jsc sources
        jsbind/jsc/jsbind.cpp
        jsbind/jsc/value.hpp
        jsbind/jsc/global.hpp
        jsbind/jsc/convert.hpp
        jsbind/jsc/call.hpp
        jsbind/jsc/bind.hpp
    )
elseif(JSBIND_V8 OR JSBIND_NODE)
    src_group(v8 sources
        jsbind/v8/jsbind.cpp
        jsbind/v8/value.hpp
        jsbind/v8/global.hpp
        jsbind/v8/convert.hpp
        jsbind/v8/call.hpp
        jsbind/v8/bind.hpp
    )
elseif(JSBIND_EMSCRIPTEN)
    src_group(em sources
        jsbind/emscripten/value.hpp
        jsbind/emscripten/bind.hpp
        jsbind/emscripten/jsbind.cpp
    )
elseif(JSBIND_CEF)
    src_group(cef sources
        jsbind/cef/jsbind.cpp
        jsbind/cef/value.hpp
        jsbind/cef/global.hpp
        jsbind/cef/convert.hpp
        jsbind/cef/call.hpp
        jsbind/cef/bind.hpp
    )
else()
    message(FATAL_ERROR "JSBind: Unsupported jsbind js engine")
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
    message(FATAL_ERROR "JSBind: Unsupported jsbind js engine")
endif()

target_compile_definitions(jsbind PUBLIC
    ${defs}
)

if(JSBIND_JS_BACKEND_LIBS)
    target_link_libraries(jsbind PUBLIC
        ${JSBIND_JS_BACKEND_LIBS}
    )
endif()
