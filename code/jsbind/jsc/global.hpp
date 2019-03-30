// jsbind
// Copyright (c) 2019 Chobolabs Inc.
// http://www.chobolabs.com/
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// http://opensource.org/licenses/MIT
//
#pragma once

#include <JavaScriptCore/JavaScript.h>

namespace jsbind
{
namespace internal
{
    extern JSGlobalContextRef jsc_context;

    extern void report_exception(JSValueRef exception);
}
}
