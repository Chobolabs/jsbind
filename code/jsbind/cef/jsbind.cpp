// jsbind
// Copyright (c) 2019 Chobolabs Inc.
// http://www.chobolabs.com/
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// http://opensource.org/licenses/MIT
//
#include "jsbind/funcs.h"
#include "jsbind/bind.h"
#include "global.h"
#include "jsbind/console.h"
#include "jsbind/exception.h"
#include "jsbind/common/deinitializers.h"

#include <sstream>
#include <iostream>

using namespace jsbind::internal;
using namespace std;

namespace
{
}

namespace jsbind
{

namespace internal
{
    CefRefPtr<CefV8Context> cef_context = nullptr;
    CefRefPtr<CefV8Value> make_uint8_array_func = nullptr;

    extern void initialize_bindings();

    CefRefPtr<CefV8Value> class_data::global = nullptr;

    void report_exception(CefRefPtr<CefV8Exception> exception)
    {
        auto eh = get_exception_handler();
        if (!eh) return;

        string msg = exception->GetMessage().ToString();
        string resource = exception->GetScriptResourceName().ToString();
        string line = exception->GetSourceLine().ToString();

        // consider using string stream here
        stringstream ss;

        ss << resource << ":" << exception->GetLineNumber() << " " << msg << endl;
        ss << "    at `" << line << "`";

        eh->on_exception(ss.str().c_str());
    }
}

void initialize()
{
    cef_context = CefV8Context::GetCurrentContext();

    set_default_exception_handler();

    // no console in cef (use browser console)

    // hacky make uint8array function
    {
        CefString code;
        code.FromASCII("(function (arrayBuf) { return new Uint8Array(arrayBuf); })");
        CefRefPtr<CefV8Exception> exception;
        cef_context->Eval(code, "jsbind.init", 0, make_uint8_array_func, exception);
    }

    // init bindings
    class_data::global = CefV8Value::CreateObject(nullptr, nullptr);

    initialize_bindings();

    auto global = cef_context->GetGlobal();

    auto str_module = to_cef_string("Module");
    global->SetValue(str_module, class_data::global, V8_PROPERTY_ATTRIBUTE_NONE);
    class_data::global = nullptr;
}

void deinitialize()
{
    internal::run_deinitializers();

    make_uint8_array_func = nullptr;

    CefV8Context* ctx = nullptr;
    cef_context.swap(&ctx);
    ctx->Release();
}

void enter_context()
{
    // devtools doesn't work if we enter and exit the context
    // who knows why...
    // it works like this, so we'll leave it for now
    //cef_context->Enter();
}

void exit_context()
{
    //cef_context->Exit();
}


void run_script(const char* src, const char* fname)
{
    CefString code;
    code.FromASCII(src);

    CefRefPtr<CefV8Value> ret;
    CefRefPtr<CefV8Exception> exception;
    auto success = cef_context->Eval(code, fname, 0, ret, exception);

    if (!success)
    {
        report_exception(exception);
    }
}

}
