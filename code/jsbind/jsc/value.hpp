// jsbind
// Copyright (c) 2019 Chobolabs Inc.
// http://www.chobolabs.com/
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// http://opensource.org/licenses/MIT
//
#pragma once

#include "convert.hpp"
#include "call.hpp"
#include <vector>
#include <functional>

#if defined(JSBIND_DEBUGGING)
extern "C" JS_EXPORT void JSGarbageCollect(JSContextRef);
#endif

namespace jsbind
{

struct scope
{
    ~scope()
    {
#if defined(JSBIND_DEBUGGING)
        JSGarbageCollect(internal::jsc_context);
#endif
    }
};

class local
{
public:
    local() = delete;

    template <typename T>
    explicit local(const T& val)
        : local(internal::to_jsc(val))
    {
    }

    static local null()
    {
        return local(JSValueMakeNull(internal::jsc_context));
    }

    static local undefined()
    {
        return local(JSValueMakeUndefined(internal::jsc_context));
    }

    static local object()
    {
        return local(JSValueRef(JSObjectMake(internal::jsc_context, nullptr, nullptr)));
    }

    static local array()
    {
        return local(JSValueRef(JSObjectMakeArray(internal::jsc_context, 0, nullptr, nullptr)));
    }

    static local global(const char* name = nullptr)
    {
        auto g = local(JSValueRef(JSContextGetGlobalObject(internal::jsc_context)));
        if (!name) return g;

        return g[name];
    }

    bool isNull() const
    {
        return JSValueIsNull(internal::jsc_context, m_handle);
    }

    bool isUndefined() const
    {
        return JSValueIsUndefined(internal::jsc_context, m_handle);
    }

    bool isTrue() const
    {
        return JSValueIsBoolean(internal::jsc_context, m_handle) && JSValueToBoolean(internal::jsc_context, m_handle) == true;
    }

    bool isFalse() const
    {
        return JSValueIsBoolean(internal::jsc_context, m_handle) && JSValueToBoolean(internal::jsc_context, m_handle) == false;
    }

    bool hasOwnProperty(const char* key) const
    {
        auto str = internal::to_jsc_string_copy(key);
        auto obj = as_jsc_object();
        bool has = JSObjectHasProperty(internal::jsc_context, obj, str);
        JSStringRelease(str);

        return has;
    }

    template <typename K>
    typename std::enable_if<std::is_integral<K>::value,
        local>::type operator[](const K& key) const
    {
        uint32_t i = uint32_t(key);
        auto obj = as_jsc_object();
        auto prop = JSObjectGetPropertyAtIndex(internal::jsc_context, obj, i, nullptr);

        return local(prop);
    }

    template <typename K>
    typename std::enable_if<!std::is_integral<K>::value,
        local>::type operator[](const K& key) const
    {
        auto str = internal::to_jsc_string_copy(key);
        auto obj = as_jsc_object();
        auto prop = JSObjectGetProperty(internal::jsc_context, obj, str, nullptr);
        JSStringRelease(str);

        return local(prop);
    }

    template <typename K, typename V>
    typename std::enable_if<std::is_integral<K>::value,
        void>::type set(const K& key, const V& val)
    {
        uint32_t i = uint32_t(key);
        auto obj = JSValueToObject(internal::jsc_context, m_handle, nullptr);
        JSObjectSetPropertyAtIndex(internal::jsc_context, obj, i, internal::to_jsc(val), nullptr);
    }

    template <typename K, typename V>
    typename std::enable_if<!std::is_integral<K>::value,
        void>::type set(const K& key, const V& val)
    {
        auto str = internal::to_jsc_string_copy(key);
        auto obj = JSValueToObject(internal::jsc_context, m_handle, nullptr);
        JSObjectSetProperty(internal::jsc_context, obj, str, internal::to_jsc(val), kJSPropertyAttributeNone, nullptr);
        JSStringRelease(str);
    }

    bool equals(const local& other) const
    {
        return JSValueIsEqual(internal::jsc_context, m_handle, other.m_handle, nullptr);
    }

    bool strictlyEquals(const local& other) const
    {
        return JSValueIsStrictEqual(internal::jsc_context, m_handle, other.m_handle);
    }

    template <typename... Args>
    local new_(const Args&... args)
    {
        const int num_args = sizeof...(Args);
        // +1 so when there are zero args we at least have something
        JSValueRef jsc_args[num_args + 1] = { internal::to_jsc(args)... };

        return local(JSValueRef(
            JSObjectCallAsConstructor(internal::jsc_context, as_jsc_object(), num_args, jsc_args, nullptr)));
    }

    template <typename... Args>
    local operator()(const Args&... args)
    {
        auto func = as_jsc_object();
        auto g = global().as_jsc_object();
        auto result = internal::jsc_call(g, func, args...);

        return local(result);
    }

    template <typename ReturnType, typename... Args>
    ReturnType call(const char* name, const Args&... args)
    {
        auto func = (*this)[name].as_jsc_object();
        auto self = as_jsc_object();
        auto result = internal::jsc_call(self, func, args...);

        return internal::from_jsc<ReturnType>(result);
    }

    template <typename T>
    T as() const
    {
        return internal::from_jsc<T>(m_handle);
    }

    local typeof() const
    {
        auto type = JSValueGetType(internal::jsc_context, m_handle);
        switch (type)
        {
        case kJSTypeUndefined: return local("undefined");
        case kJSTypeBoolean: return local("boolean");
        case kJSTypeNumber: return local("number");
        case kJSTypeString: return local("string");
        case kJSTypeObject:
        {
            auto obj = JSValueToObject(internal::jsc_context, m_handle, nullptr);
            if (JSObjectIsFunction(internal::jsc_context, obj))
            {
                return local("function");
            }
            else
            {
                return local("object");
            }
        }
        case kJSTypeNull:
        default:
            return local("object");
        }
    }

    //////////////////////////////////////////////////////////////////////
    // jsc specific data
    explicit local(JSValueRef val)
        : m_handle(val)
    {}

    JSValueRef m_handle;

    JSObjectRef as_jsc_object() const
    {
        return JSValueToObject(internal::jsc_context, m_handle, nullptr);
    }

private:
    friend class persistent;
};

template<typename T>
std::vector<T> vecFromJSArray(local v) {
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
    auto jsc_obj = obj.as_jsc_object();
    auto names = JSObjectCopyPropertyNames(internal::jsc_context, jsc_obj);

    auto len = JSPropertyNameArrayGetCount(names);

    for (size_t i = 0; i < len; ++i)
    {
        auto key = JSPropertyNameArrayGetNameAtIndex(names, i);
        auto value = JSObjectGetProperty(internal::jsc_context, jsc_obj, key, nullptr);

        auto keyVal = JSValueMakeString(internal::jsc_context, key);
        if (!iteration(local(keyVal), local(value)))
        {
            break;
        }
    }

    JSPropertyNameArrayRelease(names);
}

namespace internal
{
    inline local convert<local>::from_jsc(JSValueRef value)
    {
        return local(value);
    };

    inline JSValueRef convert<local>::to_jsc(local value)
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

    ~persistent()
    {
        if (m_handle) JSValueUnprotect(internal::jsc_context, m_handle);
    }

    void reset()
    {
        if (m_handle) JSValueUnprotect(internal::jsc_context, m_handle);
        m_handle = nullptr;
    }

    void reset(const local& l)
    {
        reset();
        m_handle = l.m_handle;
        JSValueProtect(internal::jsc_context, m_handle);
    }

    local to_local() const
    {
        return local(m_handle);
    }

    bool is_empty() const
    {
        return m_handle == nullptr;
    }

    persistent& operator=(const persistent& other)
    {
        if (m_handle) JSValueUnprotect(internal::jsc_context, m_handle);
        m_handle = other.m_handle;
        JSValueProtect(internal::jsc_context, m_handle);
        return *this;
    }

private:
    explicit persistent(JSValueRef val)
        : m_handle(val)
    {
        JSValueProtect(internal::jsc_context, m_handle);
    }

    JSValueRef m_handle;
};

}
