#pragma once

#include "value.h"

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

class uint8_array
{
public:
    uint8_array(size_t size)
        : m_size(size)
    {

#if defined(JSBIND_NOOP_TYPED_ARRAYS)
    m_buffer = new uint8_t[size];
    auto obj = local::null();
#elif defined(JSBIND_JSC) && !defined(_JSC_TYPED_ARRAYS)
    auto obj = JSValueRef(JSTypedArrayMake(internal::jsc_context, kJSTypedArrayTypeUint8Array, size));
    m_buffer = reinterpret_cast<uint8_t*>(JSTypedArrayGetDataPtr(internal::jsc_context, obj, &m_size));
#else
    m_buffer = new uint8_t[size];

#   if defined(JSBIND_V8)
    auto arrayBuffer = v8::ArrayBuffer::New(v8::Isolate::GetCurrent(), m_buffer, size);
    auto obj = v8::Uint8Array::New(arrayBuffer, 0, size);
#   elif defined(JSBIND_JSC)
    auto obj = JSValueRef(JSObjectMakeTypedArrayWithBytesNoCopy(internal::jsc_context,
        kJSTypedArrayTypeUint8Array, m_buffer, size, [](void*, void*){}, nullptr, nullptr));
#   elif defined(JSBIND_EMSCRIPTEN)
    auto obj = emscripten::typed_memory_view(size, m_buffer);
#   elif defined(JSBIND_CEF)
    auto arrayBuf = CefV8Value::CreateArrayBuffer(m_buffer, size, new internal::release_buffer_callback);
    auto obj = internal::make_uint8_array_func->ExecuteFunction(nullptr, { arrayBuf });
#   endif

#endif

    m_persistent.reset(local(obj));
    }

    uint8_array(uint8_array&& other)
        : m_size(other.m_size)
        , m_buffer(other.m_buffer)
        , m_persistent(std::move(other.m_persistent))
    {
        other.m_size = 0;
        other.m_buffer = nullptr;
        other.m_persistent.reset();
    }

    ~uint8_array()
    {
#if !defined(JSBIND_JSC) || defined(_JSC_TYPED_ARRAYS) || defined(JSBIND_NOOP_TYPED_ARRAYS)
        delete[] m_buffer;
#endif
    }

    size_t get_size() const { return m_size; }

    const void* get_buffer() const { return m_buffer; }
    uint8_t* get_buffer() { return m_buffer; }

    const persistent& get_persistent() const { return m_persistent; }

private:
    size_t m_size;
    uint8_t* m_buffer;
    persistent m_persistent;
};

class array_buffer
{
public:
    array_buffer(size_t size)
        : m_size(size)
        , m_buffer(new uint8_t[size])
        , m_owner(true)
    {
        init();
    }

    array_buffer(uint8_t* buf, size_t size)
        : m_size(size)
        , m_buffer(buf)
        , m_owner(false)
    {
        init();
    }

    ~array_buffer()
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

private:

    void init()
    {
#if defined(JSBIND_EMSCRIPTEN)
        auto u8a = local(emscripten::typed_memory_view(m_size, m_buffer));
        auto obj = u8a["buffer"];
#elif defined(JSBIND_CEF)
        auto obj = CefV8Value::CreateArrayBuffer(m_buffer, m_size, new internal::release_buffer_callback);
#else
#   error "jsbind: Not supported platform (yet)"
#endif
        m_persistent.reset(local(obj));
    }

    size_t m_size;
    uint8_t* m_buffer;
    persistent m_persistent;
    bool m_owner;
};

}
