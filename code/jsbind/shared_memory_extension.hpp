#pragma once

#include "value.hpp"

#if defined(JSBIND_V8)
#elif defined(JSBIND_JSC)
#include <JavaScriptCore/JSTypedArray.h>
#elif defined(JSBIND_EMSCRIPTEN)
#   include "emscripten/wire.h"
#elif defined(JSBIND_CEF)
#else
#   error "jsbind: Unknown platform"
#endif

#include <cstdint>

namespace jsbind
{

namespace internal
{
template <typename T>
class buffer
{
public:
    buffer(size_t size)
        : m_size(size)
        , m_buffer(new uint8_t[size])
        , m_owner(true)
    {
        static_cast<T*>(this)->init();
    }

    buffer(uint8_t* buf, size_t size)
        : m_size(size)
        , m_buffer(buf)
        , m_owner(false)
    {
        static_cast<T*>(this)->init();
    }

    buffer(buffer&& other)
        : m_size(other.m_size)
        , m_buffer(other.m_buffer)
        , m_persistent(std::move(other.m_persistent))
        , m_owner(other.m_owner)
    {
        other.m_size = 0;
        other.m_buffer = nullptr;
        other.m_persistent.reset();
        other.m_owner = true;
    }

    buffer(const buffer&) = delete;
    buffer operator=(const buffer&) = delete;

    ~buffer()
    {
        if (m_owner)
        {
            delete[] m_buffer;
        }
    }

    size_t get_size() const { return m_size; }

    const void* get_buffer() const { return m_buffer; }
    uint8_t* get_buffer() { return m_buffer; }

    const persistent& get_persistent() const { return m_persistent; }

protected:
    size_t m_size;
    uint8_t* m_buffer;
    persistent m_persistent;
    bool m_owner;
};
}

#if defined(JSBIND_CEF)
namespace internal
{
class release_buffer_callback : public CefV8ArrayBufferReleaseCallback
{
    virtual void ReleaseBuffer(void*) override {}
    IMPLEMENT_REFCOUNTING(release_buffer_callback)
};
}
#endif

class uint8_array : public internal::buffer<uint8_array>
{
public:
    uint8_array(size_t size) : buffer(size) {}
    uint8_array(uint8_t* buf, size_t size) : buffer(buf, size) {}

    void init()
    {
#if defined(JSBIND_NOOP_TYPED_ARRAYS)
        auto obj = local::null();
#elif defined(JSBIND_V8)
        auto arrayBuffer = v8::ArrayBuffer::New(v8::Isolate::GetCurrent(), m_buffer, m_size);
        auto obj = v8::Uint8Array::New(arrayBuffer, 0, m_size);
#elif defined(JSBIND_JSC)
        auto obj = JSValueRef(JSObjectMakeTypedArrayWithBytesNoCopy(internal::jsc_context,
            kJSTypedArrayTypeUint8Array, m_buffer, m_size, [](void*, void*){}, nullptr, nullptr));
#elif defined(JSBIND_EMSCRIPTEN)
        auto obj = emscripten::typed_memory_view(m_size, m_buffer);
#elif defined(JSBIND_CEF)
        auto arrayBuf = CefV8Value::CreateArrayBuffer(m_buffer, m_size, new internal::release_buffer_callback);
        auto obj = internal::make_uint8_array_func->ExecuteFunction(nullptr, { arrayBuf });
#endif
        m_persistent.reset(local(obj));
    }
};

class array_buffer : public internal::buffer<array_buffer>
{
public:
    array_buffer(size_t size) : buffer(size) {}
    array_buffer(uint8_t* buf, size_t size) : buffer(buf, size) {}

    void init()
    {
#if defined(JSBIND_NOOP_TYPED_ARRAYS)
        auto obj = local::null();
#elif defined(JSBIND_V8)
        auto obj = v8::ArrayBuffer::New(v8::Isolate::GetCurrent(), m_buffer, m_size);
#elif defined(JSBIND_JSC)
        auto obj = JSValueRef(JSObjectMakeTypedArrayWithBytesNoCopy(internal::jsc_context,
            kJSTypedArrayTypeArrayBuffer, m_buffer, m_size, [](void*, void*){}, nullptr, nullptr));
#elif defined(JSBIND_EMSCRIPTEN)
        auto u8a = local(emscripten::typed_memory_view(m_size, m_buffer));
        auto obj = u8a["buffer"];
#elif defined(JSBIND_CEF)
        auto obj = CefV8Value::CreateArrayBuffer(m_buffer, m_size, new internal::release_buffer_callback);
#endif
        m_persistent.reset(local(obj));
    }
};

}
