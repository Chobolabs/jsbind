// jsbind
// Copyright (c) 2019 Chobolabs Inc.
// http://www.chobolabs.com/
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// http://opensource.org/licenses/MIT
//
#pragma once

#if !defined(NOMINMAX)
#   define NOMINMAX
#endif
#if defined(__GNUC__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif
#include <include/cef_v8.h>
#if defined(__GNUC__)
#pragma GCC diagnostic pop
#endif

namespace jsbind
{

namespace internal
{
    extern CefRefPtr<CefV8Context> cef_context;
    extern CefRefPtr<CefV8Value> make_uint8_array_func;

    extern void report_exception(CefRefPtr<CefV8Exception> exception);
}

}
