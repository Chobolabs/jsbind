// jsbind
// Copyright (c) 2019 Chobolabs Inc.
// http://www.chobolabs.com/
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// http://opensource.org/licenses/MIT
//
#pragma once

#include <emscripten/bind.h>

namespace jsbind
{
    using emscripten::function;
    using emscripten::class_;
    using emscripten::value_object;
}

#define JSBIND_BINDINGS EMSCRIPTEN_BINDINGS
