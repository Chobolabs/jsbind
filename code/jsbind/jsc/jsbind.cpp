// jsbind
// Copyright (c) 2019 Chobolabs Inc.
// http://www.chobolabs.com/
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// http://opensource.org/licenses/MIT
//
#include "jsbind/funcs.hpp"
#include "convert.hpp"
#include "bind.hpp"
#include "jsbind/console.hpp"
#include "jsbind/exception.hpp"
#include "jsbind/common/deinitializers.hpp"

#include <sstream>
#include <iostream>
#include <vector>

using namespace jsbind::internal;

namespace
{

JSValueRef jsc_console_msg(jsbind::console::msg_type type, size_t num_args, const JSValueRef args[])
{
    auto undefined = JSValueMakeUndefined(jsbind::internal::jsc_context);

    auto con = jsbind::get_console();
    if (!con) return undefined;

    std::ostringstream sout;
    for (size_t i = 0; i < num_args; ++i)
    {
        if (i) sout << ' ';

        sout << from_jsc<std::string>(args[i]);
    }

    con->msg(type, sout.str().c_str());

    return undefined;
}

JSValueRef jsc_console_log(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
    return jsc_console_msg(jsbind::console::msg_log, argumentCount, arguments);
}

JSValueRef jsc_console_info(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
    return jsc_console_msg(jsbind::console::msg_info, argumentCount, arguments);
}

JSValueRef jsc_console_warn(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
    return jsc_console_msg(jsbind::console::msg_warn, argumentCount, arguments);
}

JSValueRef jsc_console_error(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
    return jsc_console_msg(jsbind::console::msg_error, argumentCount, arguments);
}

JSValueRef jsc_console_debug(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
    return jsc_console_msg(jsbind::console::msg_debug, argumentCount, arguments);
}

JSValueRef jsc_console_assert(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
    if (argumentCount == 0) return JSValueMakeUndefined(jsbind::internal::jsc_context);

    if (JSValueToBoolean(jsbind::internal::jsc_context, arguments[0]))
    {
        return JSValueMakeUndefined(jsbind::internal::jsc_context);
    }

    return jsc_console_msg(jsbind::console::msg_assert, argumentCount - 1, arguments + 1);
}

}

namespace jsbind
{

namespace internal
{
    JSGlobalContextRef jsc_context;
    extern void initialize_bindings();

    JSObjectRef class_data::global = nullptr;
    JSClassRef class_data::function_class = nullptr;

    void report_exception(JSValueRef exception)
    {
        auto eh = get_exception_handler();
        if (!eh) return;

        std::stringstream sout;

        JSStringRef jsLinePropertyName = JSStringCreateWithUTF8CString("line");
        JSStringRef jsFilePropertyName = JSStringCreateWithUTF8CString("sourceURL");
        JSStringRef jsStackTraceName = JSStringCreateWithUTF8CString("stack");

        JSObjectRef exObject = JSValueToObject(jsc_context, exception, NULL);
        JSValueRef line = JSObjectGetProperty(jsc_context, exObject, jsLinePropertyName, NULL);
        JSValueRef file = JSObjectGetProperty(jsc_context, exObject, jsFilePropertyName, NULL);

        auto ex = from_jsc<std::string>(exception);
        sout << ex << " at " << from_jsc<std::string>(line) << " in " << from_jsc<std::string>(file) << std::endl;
        sout << "Stack trace:" << std::endl;

        JSValueRef stackTrace = JSObjectGetProperty(jsc_context, exObject, jsStackTraceName, NULL);
        sout << ex << std::endl;
        sout << from_jsc<std::string>(stackTrace) << std::endl;

        JSStringRelease(jsLinePropertyName);
        JSStringRelease(jsFilePropertyName);
        JSStringRelease(jsStackTraceName);

        eh->on_exception(sout.str().c_str());
    }
}

void initialize()
{
    jsc_context = JSGlobalContextCreate(nullptr);

    set_default_exception_handler();

    // create custom classes
    {
        JSClassDefinition js_def = kJSClassDefinitionEmpty;
        js_def.attributes = kJSClassAttributeNoAutomaticPrototype;
        js_def.className = "JSBindFunction";
        js_def.callAsFunction = call_from_jsc;
        js_def.finalize = [](JSObjectRef f)
        {
            delete reinterpret_cast<function_private_data*>(JSObjectGetPrivate(f));
        };
        class_data::function_class = JSClassCreate(&js_def);
    }

    // init console
    set_default_console();
    JSClassDefinition console_def = kJSClassDefinitionEmpty;
    JSStaticFunction console_funcs[] = {
        { "log", jsc_console_log, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
        { "info", jsc_console_info, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
        { "warn", jsc_console_warn, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
        { "error", jsc_console_error, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
        { "debug", jsc_console_debug, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
        { "assert", jsc_console_assert, kJSPropertyAttributeDontDelete },
        {} // null termination
    };

    console_def.staticFunctions = console_funcs;
    auto console_class = JSClassCreate(&console_def);

    // init bindings
    class_data::global = JSObjectMake(internal::jsc_context, nullptr, nullptr);
    JSValueProtect(internal::jsc_context, class_data::global);

    initialize_bindings();

    auto global = JSContextGetGlobalObject(jsc_context);

    auto str_console = to_jsc_string_copy("console");
    JSObjectRef console_obj = JSObjectMake(internal::jsc_context, console_class, nullptr);
    JSObjectSetProperty(internal::jsc_context, global, str_console, console_obj, kJSPropertyAttributeNone, nullptr);
    JSStringRelease(str_console);

    auto str_module = to_jsc_string_copy("Module");
    JSObjectSetProperty(internal::jsc_context, global, str_module, class_data::global, kJSPropertyAttributeNone, nullptr);
    JSStringRelease(str_module);

    JSValueUnprotect(internal::jsc_context, class_data::global);
    class_data::global = nullptr;
}

void deinitialize()
{
    internal::run_deinitializers();

    JSGlobalContextRelease(jsc_context);
    jsc_context = nullptr;
}

void enter_context()
{
}

void exit_context()
{
}

extern "C" JS_EXPORT void JSGarbageCollect(JSContextRef);

void run_script(const char* src, const char* fname)
{
    auto source = JSStringCreateWithUTF8CString(src);
    auto filename = fname ? JSStringCreateWithUTF8CString(fname) : nullptr;

    if (source)
    {
        JSValueRef exception = nullptr;
        JSEvaluateScript(jsc_context, source, nullptr, filename, 0, &exception);

        if (exception)
        {
            report_exception(exception);
        }
        JSStringRelease(source);
        if (filename) JSStringRelease(filename);
    }
}

}
