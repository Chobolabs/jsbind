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

#if defined(JSBIND_NODE)
#include <node.h>
#endif

#include <v8.h>

#if !defined(JSBIND_NODE)
#include <libplatform/libplatform.h>
#endif

#include <cstdlib> // malloc
#include <cstring> // memset

#include <sstream>
#include <iostream>

using namespace v8;
using namespace jsbind::internal;
using namespace std;

#if !defined(JSBIND_NODE)

namespace
{
class MallocArrayBufferAllocator : public ArrayBuffer::Allocator
{
private:
    size_t m_bytes_allocated;

public:
    MallocArrayBufferAllocator() : m_bytes_allocated(0) {}

    virtual void* Allocate(size_t length)
    {
        void *mem = malloc(length);
        memset(mem, 0, length);
        m_bytes_allocated += length;
        return mem;
    }

    virtual void* AllocateUninitialized(size_t length)
    {
        void *mem = malloc(length);
        memset(mem, 0, length);
        m_bytes_allocated += length;
        return mem;
    }
    virtual void Free(void* data, size_t len)
    {
        m_bytes_allocated -= len;
        //        logInfo("Total allocations: %d", m_BytesAllocated);
        free(data);
    }
};

// Android-friendly way of initializing v8
// Since we can't know when our process is really closed
// (Android reuses live instancec of our .so at will)
// We cannot have a safe place to uninitialize v8
// Also, once you go V8::Dispose(), you never go back
// It destroys global internal data, and there is no way to recover
// So we have a static variable of this type, and it will intialize itself globally
// and then, when our process really, it will dispose of v8 properly at the right time
struct V8Initializer
{
    V8Initializer()
    {
        V8::InitializeICU();
        platform = platform::CreateDefaultPlatform();
        V8::InitializePlatform(platform);
        V8::Initialize();
    }

    ~V8Initializer()
    {
        V8::Dispose();
        V8::ShutdownPlatform();
        delete platform;
        platform = nullptr;
    }

    Platform* platform;

    MallocArrayBufferAllocator allocator;
};

void v8_msg(jsbind::console::msg_type type, int startArg, const v8::FunctionCallbackInfo<v8::Value>& args)
{
    auto con = jsbind::get_console();
    if (!con) return;

    std::ostringstream sout;
    for (int i = startArg; i < args.Length(); ++i)
    {
        if (i != startArg) sout << ' ';
        sout << from_v8<std::string>(args[i]);
    }

    con->msg(type, sout.str().c_str());
}

void v8_log(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    v8_msg(jsbind::console::msg_log, 0, args);
}

void v8_info(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    v8_msg(jsbind::console::msg_info, 0, args);
}

void v8_warn(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    v8_msg(jsbind::console::msg_warn, 0, args);
}

void v8_error(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    v8_msg(jsbind::console::msg_error, 0, args);
}

void v8_debug(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    v8_msg(jsbind::console::msg_debug, 0, args);
}

void v8_assert(const v8::FunctionCallbackInfo<v8::Value>& args)
{
    if (args.Length() == 0) return;

    if (args[0].As<v8::Boolean>()->BooleanValue())
    {
        return;
    }

    v8_msg(jsbind::console::msg_assert, 1, args);
}

}

#endif

namespace jsbind
{

namespace internal
{
    v8::Isolate* isolate = nullptr;
    context ctx;
    extern void initialize_bindings();

    v8::Local<v8::ObjectTemplate>* class_data::global = nullptr;

    void report_exception(const v8::TryCatch& tryCatch)
    {
        auto eh = get_exception_handler();
        if (!eh) return;

        v8::HandleScope handleScope(isolate);

        v8::String::Utf8Value exception(tryCatch.Exception());
        const char* exceptionString = *exception;
        v8::Handle<v8::Message> message = tryCatch.Message();

        stringstream ss;

        if (message.IsEmpty())
        {
            // V8 didn't provide any extra information about this error; just
            // print the exception.
            ss << exceptionString;
        }
        else
        {
            // Print (filename):(line number): (message).
            v8::String::Utf8Value filename(message->GetScriptResourceName());
            const char* filenameString = *filename;
            int linenum = message->GetLineNumber();
            ss << filenameString << ":" << linenum << ": " << exceptionString << std::endl;

            // Print line of source code.
            v8::String::Utf8Value sourceline(message->GetSourceLine());
            const char* sourceline_string = *sourceline;
            ss << sourceline_string << std::endl;

            // Print wavy underline (GetUnderline is deprecated).
            int start = message->GetStartColumn();

            for (int i = 0; i < start; i++)
            {
                ss << " ";
            }
            int end = message->GetEndColumn();
            for (int i = start; i < end; i++)
            {
                ss << "^";
            }
            ss << std::endl;

            v8::String::Utf8Value stack_trace(tryCatch.StackTrace());
            if (stack_trace.length() > 0) {
                const char* stack_trace_string = *stack_trace;
                ss << stack_trace_string;
            }
        }

        eh->on_exception(ss.str().c_str());
    }

    void report_fatal_error(const char* location, const char* message)
    {
        auto eh = get_exception_handler();
        if (!eh) return;

        stringstream ss;
        ss << location << ": " << message;
        eh->on_engine_error(ss.str().c_str());
    }
}

#if !defined(JSBIND_NODE)
void initialize()
{
    static V8Initializer initializer;

    Isolate::CreateParams params;
    params.array_buffer_allocator = &initializer.allocator;
    isolate = Isolate::New(params);

    {
        v8::Locker lock(isolate);
        v8::HandleScope scope(isolate);


        v8::Local<v8::ObjectTemplate> global = v8::ObjectTemplate::New(isolate);

        // console
        set_default_console();
        v8::Local<v8::ObjectTemplate> console_obj = v8::ObjectTemplate::New(isolate);
        console_obj->Set(isolate, "log", v8::FunctionTemplate::New(isolate, v8_log));
        console_obj->Set(isolate, "info", v8::FunctionTemplate::New(isolate, v8_info));
        console_obj->Set(isolate, "warn", v8::FunctionTemplate::New(isolate, v8_warn));
        console_obj->Set(isolate, "error", v8::FunctionTemplate::New(isolate, v8_error));
        console_obj->Set(isolate, "debug", v8::FunctionTemplate::New(isolate, v8_debug));
        console_obj->Set(isolate, "assert", v8::FunctionTemplate::New(isolate, v8_assert));
        global->Set(isolate, "console", console_obj);

        // bindings
        v8::Local<v8::ObjectTemplate> module = v8::ObjectTemplate::New(isolate);
        class_data::global = &module;
        initialize_bindings();
        global->Set(isolate, "Module", module);

        class_data::global = nullptr;

        v8::Local<v8::Context> c = v8::Context::New(isolate, nullptr, global);

        ctx.v8ctx.Reset(isolate, c);

        ctx.enter();
        v8::V8::SetFatalErrorHandler(report_fatal_error);
        ctx.exit();
    }
}
#endif

void v8_initialize_with_global(v8::Local<Object> global)
{
    // intentionally not setting the fatal error handler
    // let the external initializer (most likely node) handle it

    isolate = v8::Isolate::GetCurrent();
    v8::HandleScope scope(isolate);
    ctx.v8ctx.Reset(isolate, isolate->GetCurrentContext());

    // bindings
    v8::Local<v8::ObjectTemplate> module = v8::ObjectTemplate::New(isolate);
    class_data::global = &module;
    initialize_bindings();
    auto r = global->SetPrototype(ctx.to_local(), module->NewInstance());
    JSBIND_JS_CHECK(r.FromMaybe(false), "Could not set Module prototype");
    class_data::global = nullptr;
}

void deinitialize()
{
    internal::run_deinitializers();

    ctx.v8ctx.Reset();
#if !defined(JSBIND_NODE)
    // node manages the isolate
    isolate->Dispose();
#endif
    isolate = nullptr;
}

void enter_context()
{
#if !defined(JSBIND_NODE)
    // node manages the context
    ctx.enter();
#endif
}

void exit_context()
{
#if !defined(JSBIND_NODE)
    // node manages the context
    ctx.exit();
#endif
}

void run_script(const char* src, const char* fname)
{
    HandleScope scope(isolate);

    auto source = String::NewFromUtf8(isolate, src, NewStringType::kNormal).ToLocalChecked();

    Local<Script> script;

    if (fname)
    {
        auto filename = String::NewFromUtf8(isolate, fname, NewStringType::kNormal).ToLocalChecked();
        script = Script::Compile(source, filename);
    }
    else
    {
        script = Script::Compile(source);
    }

    v8::TryCatch try_catch(isolate);

    if (script.IsEmpty())
    {
        // Print errors that happened during compilation.
        report_exception(try_catch);
        return;
    }
    else
    {
        v8::Handle<v8::Value> result = script->Run();

        if (result.IsEmpty())
        {
            report_exception(try_catch);
            return;
        }
    }
}

}
