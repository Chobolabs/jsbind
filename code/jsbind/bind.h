// jsbind
// Copyright (c) 2019 Chobolabs Inc.
// http://www.chobolabs.com/
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// http://opensource.org/licenses/MIT
//
#pragma once

#if defined(JSBIND_V8)
#   include "v8/bind.h"
#elif defined(JSBIND_JSC)
#   include "jsc/bind.h"
#elif defined(JSBIND_EMSCRIPTEN)
#   include "emscripten/bind.h"
#elif defined(JSBIND_CEF)
#   include "cef/bind.h"
#else
#   error "Unknown platform"
#endif

#if !defined(JSBIND_EMSCRIPTEN)
#include <deque>

namespace jsbind
{
namespace internal
{
    struct binding_initializer
    {
        binding_initializer();

        static std::deque<binding_initializer*>& instances();

        virtual void initialize() = 0;
    };
}
}

#define JSBIND_BINDINGS(name) \
    static struct _jsbind_binding_initializer_##name : public jsbind::internal::binding_initializer { \
        virtual void initialize() override; \
    } _jsbind_static_initializer_##name; \
    void _jsbind_binding_initializer_##name::initialize()

#endif
