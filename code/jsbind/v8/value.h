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
    scope()
        : m_scope(internal::isolate)
    {}
private:
    v8::HandleScope m_scope;
};

class local
{
public:
    local() = delete;


    //////////////////////////////////////////////
    // v8 specific
    template <typename Value>
    explicit local(const v8::Local<Value>& val)
    : m_handle(val)
    {}

    v8::Local<v8::Value> m_handle;


    template <typename T>
    explicit local(const T& val)
        : local(internal::to_v8(val))
    {
    }

    static local null()
    {
        return local(v8::Null(internal::isolate));
    }

    static local undefined()
    {
        return local(v8::Undefined(internal::isolate));
    }

    static local object()
    {
        return local(v8::Object::New(internal::isolate));
    }

    static local array()
    {
        return local(v8::Array::New(internal::isolate));
    }

    static local global(const char* name = nullptr)
    {
        auto& v8ctx = *reinterpret_cast<v8::Local<v8::Context>*>(&internal::ctx.v8ctx);
        local g = local(v8ctx->Global());

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
        return m_handle->IsTrue();
    }

    bool isFalse() const
    {
        return m_handle->IsFalse();
    }

    bool hasOwnProperty(const char* key) const
    {
        auto k = internal::to_v8(key);
        auto obj = v8::Object::Cast(*m_handle);

        auto maybe = obj->HasOwnProperty(internal::ctx.to_local(), k);
        return maybe.FromMaybe(false);
    }

    template <typename K>
    typename std::enable_if<std::is_integral<K>::value,
        local>::type operator[](const K& key) const
    {
        uint32_t i = uint32_t(key);
        auto obj = v8::Object::Cast(*m_handle);

        return local(obj->Get(i));
    }

    template <typename K>
    typename std::enable_if<!std::is_integral<K>::value,
        local>::type operator[](const K& key) const
    {
        auto k = internal::to_v8(key);
        auto obj = v8::Object::Cast(*m_handle);

        return local(obj->Get(k));
    }

    template <typename K, typename V>
    typename std::enable_if<std::is_integral<K>::value,
        void>::type set(const K& key, const V& val)
    {
        uint32_t i = uint32_t(key);
        auto obj = v8::Object::Cast(*m_handle);

        obj->Set(i, internal::to_v8(val));
    }

    template <typename K, typename V>
    typename std::enable_if<!std::is_integral<K>::value,
        void>::type set(const K& key, const V& val)
    {
        auto k = internal::to_v8(key);
        auto obj = v8::Object::Cast(*m_handle);

        obj->Set(k, internal::to_v8(val));
    }

    bool equals(const local& other) const
    {
        return m_handle->Equals(other.m_handle);
    }

    bool strictlyEquals(const local& other) const
    {
        return m_handle->StrictEquals(other.m_handle);
    }

    template <typename... Args>
    local new_(const Args&... args)
    {
        v8::EscapableHandleScope scope(internal::isolate);

        auto obj = v8::Object::Cast(*m_handle);

        const int num_args = sizeof...(Args);
        // +1 so when there are zero args we at least have something
        v8::Local<v8::Value> v8_args[num_args + 1] = { internal::to_v8(args)... };

        auto maybe = obj->CallAsConstructor(internal::ctx.to_local(), num_args, v8_args);
        return local(scope.Escape(maybe.FromMaybe(v8::Local<v8::Value>())));
    }

    template <typename... Args>
    local operator()(const Args&... args)
    {
        auto func = v8::Function::Cast(*m_handle);
        return local(internal::v8_call(global().m_handle, func, args...));
    }

    template <typename ReturnType, typename... Args>
    ReturnType call(const char* name, const Args&... args)
    {
        auto func = v8::Function::Cast(*(*this)[name].m_handle);
        auto result = internal::v8_call(m_handle, func, args...);
        return internal::from_v8<ReturnType>(result);
    }

    template <typename T>
    T as() const
    {
        return internal::from_v8<T>(m_handle);
    }

    local typeOf() const
    {
        // return local(m_handle->TypeOf(isolate));
        if (m_handle->IsUndefined()) return local("undefined");
        if (m_handle->IsBoolean()) return local("boolean");
        if (m_handle->IsNumber()) return local("number");
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
    auto v8_obj = v8::Object::Cast(*obj.m_handle);
    auto names = v8_obj->GetPropertyNames();
    auto len = names->Length();

    for (uint32_t i = 0; i < len; ++i)
    {
        auto key = names->Get(i);
        auto val = v8_obj->Get(key);

        if (!iteration(local(key), local(val)))
        {
            break;
        }
    }
}

namespace internal
{
    inline local convert<local>::from_v8(v8::Local<v8::Value> value)
    {
        return local(value);
    }

    inline v8::Local<v8::Value> convert<local>::to_v8(const local& value)
    {
        return value.m_handle;
    }
}

class persistent
{
public:
    persistent()
    {}

    explicit persistent(const local& local)
        : persistent(local.m_handle)
    {}

    void reset()
    {
        m_handle.Reset();
    }

    void reset(const local& l)
    {
        m_handle.Reset(internal::isolate, l.m_handle);
    }

    local to_local() const
    {
        // not thread safe but fast
        // perhaps add if (m_handle.IsWeak()) return v8::Local<T>::New(isolate, m_handle);
        return *reinterpret_cast<const local*>(&m_handle);
    }

    bool is_empty() const
    {
        return m_handle.IsEmpty();
    }

private:
    explicit persistent(v8::Local<v8::Value> val)
        : m_handle(internal::isolate, val)
    {}

    v8::CopyablePersistentTraits<v8::Value>::CopyablePersistent m_handle;
};

}
