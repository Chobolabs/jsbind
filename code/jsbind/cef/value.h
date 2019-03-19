// jsbind
// Copyright (c) 2019 Chobolabs Inc.
// http://www.chobolabs.com/
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// http://opensource.org/licenses/MIT
//
#pragma once

#include "convert.h"
#include "call.h"

#include <vector>
#include <functional>

namespace jsbind
{

class scope
{
public:
    scope() {}
    ~scope() {}
};

class local
{
public:
    local() = delete;

    //////////////////////////////////////////////
    // cef specific
    explicit local(CefRefPtr<CefV8Value> val)
        : m_handle(val)
    {}

    CefRefPtr<CefV8Value> m_handle;

    template <typename T>
    explicit local(const T& val)
        : local(internal::to_cef(val))
    {
    }

    static local null()
    {
        return local(CefV8Value::CreateNull());
    }

    static local undefined()
    {
        return local(CefV8Value::CreateUndefined());
    }

    static local object()
    {
        return local(CefV8Value::CreateObject(nullptr, nullptr));
    }

    static local array()
    {
        return local(CefV8Value::CreateArray(0));
    }

    static local global(const char* name = nullptr)
    {
        local g = local(internal::cef_context->GetGlobal());

        if (!name) return g;

        return g[name];
    }

    bool isNull() const
    {
        return m_handle->IsNull();
    }

    bool isUndefined() const
    {
        return m_handle->IsUndefined();
    }

    bool isTrue() const
    {
        return m_handle->GetBoolValue();
    }

    bool isFalse() const
    {
        return !m_handle->GetBoolValue();
    }

    bool hasOwnProperty(const char* key) const
    {
        CefString str;
        str.FromASCII(key);
        return m_handle->HasValue(str);
    }

    template <typename K>
    typename std::enable_if<std::is_integral<K>::value,
        local>::type operator[](const K& key) const
    {
        int i = int(key);
        return local(m_handle->GetValue(i));
    }

    template <typename K>
    typename std::enable_if<!std::is_integral<K>::value,
        local>::type operator[](const K& key) const
    {
        auto k = internal::to_cef_string(key);
        return local(m_handle->GetValue(k));
    }

    template <typename K, typename V>
    typename std::enable_if<std::is_integral<K>::value,
        void>::type set(const K& key, const V& val)
    {
        int i = int(key);
        m_handle->SetValue(i, internal::to_cef(val));
    }

    template <typename K, typename V>
    typename std::enable_if<!std::is_integral<K>::value,
        void>::type set(const K& key, const V& val)
    {
        auto k = internal::to_cef_string(key);
        m_handle->SetValue(k, internal::to_cef(val), V8_PROPERTY_ATTRIBUTE_NONE);
    }

    bool equals(const local& /*other*/) const
    {
        assert(false && "Cannot be implemented");
        return false;
    }

    bool strictlyEquals(const local& /*other*/) const
    {
        assert(false && "Cannot be implemented");
        return false;
    }

    template <typename... Args>
    local new_(const Args&... /*args*/)
    {
        assert(false && "Cannot be implemented");
        return undefined();
    }

    template <typename... Args>
    local operator()(const Args&... args)
    {
        return local(internal::cef_call(global().m_handle, m_handle, args...));
    }

    template <typename ReturnType, typename... Args>
    ReturnType call(const char* name, const Args&... args)
    {
        auto func = (*this)[name].m_handle;
        auto result = internal::cef_call(m_handle, func, args...);
        return internal::from_cef<ReturnType>(result);
    }

    template <typename T>
    T as() const
    {
        return internal::from_cef<T>(m_handle);
    }

    local typeOf() const
    {
        if (m_handle->IsUndefined()) return local("undefined");
        if (m_handle->IsBool()) return local("boolean");
        if (m_handle->IsDouble() || m_handle->IsInt() || m_handle->IsUInt()) return local("number");
        if (m_handle->IsString()) return local("string");
        if (m_handle->IsFunction()) return local("function");
        return local("object");
    }


private:

    friend class persistent;
};

template<typename T>
std::vector<T> vecFromJSArray(local v)
{
    auto l = v["length"].as<unsigned>();

    std::vector<T> ret;
    ret.reserve(l);
    for (unsigned i = 0; i < l; ++i) {
        ret.emplace_back(v[i].as<T>());
    }

    return ret;
};

inline void foreach(local obj, std::function<bool(local key, local value)> iteration)
{
    std::vector<CefString> keys;
    obj.m_handle->GetKeys(keys);

    for (auto& key : keys)
    {
        auto val = obj.m_handle->GetValue(key);

        if (!iteration(local(CefV8Value::CreateString(key)), local(val)))
        {
            break;
        }
    }
}

namespace internal
{
    inline local convert<local>::from_cef(CefRefPtr<CefV8Value> value)
    {
        return local(value);
    }

    inline CefRefPtr<CefV8Value> convert<local>::to_cef(const local& value)
    {
        return value.m_handle;
    }
}


class persistent
{
public:
    persistent()
        : m_handle(nullptr)
    {}

    persistent(const persistent& other)
        : persistent(other.m_handle)
    {}

    persistent(persistent&& other)
        : m_handle(other.m_handle)
    {
        other.m_handle = nullptr;
    }

    explicit persistent(const local& local)
        : persistent(local.m_handle)
    {}

    void reset()
    {
        m_handle = nullptr;
    }

    void reset(const local& l)
    {
        m_handle = l.m_handle;
    }

    local to_local() const
    {
        return local(m_handle);
    }

    bool is_empty() const
    {
        return !m_handle;
    }

    persistent& operator=(const persistent& other)
    {
        m_handle = other.m_handle;
        return *this;
    }

private:
    explicit persistent(CefRefPtr<CefV8Value> val)
        : m_handle(val)
    {
    }

    CefRefPtr<CefV8Value> m_handle;
};

}
