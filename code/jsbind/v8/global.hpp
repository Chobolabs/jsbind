// jsbind
// Copyright (c) 2019 Chobolabs Inc.
// http://www.chobolabs.com/
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// http://opensource.org/licenses/MIT
//
#pragma once

#if defined(JSBIND_NODE)
#   include <node.h>
#endif

#include <v8.h>

#include <new>

namespace jsbind
{

extern void v8_initialize_with_global(v8::Local<v8::Object> global);

namespace internal
{
    extern v8::Isolate* isolate;

    struct context
    {
        v8::Locker* m_locker = nullptr;

        void enter()
        {
            if (!m_locker) // simple reentry
            {
                m_locker = new (buf)v8::Locker(isolate);

                isolate->Enter();

                auto ctx = to_local();
                ctx->Enter();
            }
        }

        void exit()
        {
            if (m_locker)
            {
                auto ctx = to_local();
                ctx->Exit();

                isolate->Exit();

                m_locker->~Locker();
                m_locker = nullptr;
            }
        }

        const v8::Local<v8::Context>& to_local() const
        {
            return *reinterpret_cast<const v8::Handle<v8::Context>*>(&v8ctx);
        }

        v8::Persistent<v8::Context> v8ctx;

        char buf[sizeof(v8::Locker)]; // placing the locker here to avoid needless allocations
    };

    extern context ctx;

    extern void report_exception(const v8::TryCatch& try_catch);
}

}
