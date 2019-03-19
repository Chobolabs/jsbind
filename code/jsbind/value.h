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
#   include "v8/value.h"
#elif defined(JSBIND_JSC)
#   include "jsc/value.h"
#elif defined(JSBIND_EMSCRIPTEN)
#   include "emscripten/value.h"
#elif defined(JSBIND_CEF)
#   include "cef/value.h"
#else
#   error "jsbind: Unknown platform"
#endif