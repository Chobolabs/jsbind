// jsbind
// Copyright (c) 2019 Chobolabs Inc.
// http://www.chobolabs.com/
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// http://opensource.org/licenses/MIT
//
#pragma once

#include "global.h"

#include "jsbind/common/wrapped_class.h"

#include <string>
#include <type_traits>
#include <climits>

namespace jsbind
{

class local;

namespace internal
{
    template <typename T, typename Enable = void> // enable used by enable_if specializations
    struct convert;

    template<>
    struct convert<const char*>
    {
        using type = const char*;

        // raw pointer converion not supported
        // it could be supported but emscripten doesn't do it, so we'll follow
        static type from_jsc(JSValueRef val) = delete;

        static JSValueRef to_jsc(const char* val)
        {
            auto str = JSStringCreateWithUTF8CString(val);
            auto ret = JSValueMakeString(jsc_context, str);

            JSStringRelease(str);

            return ret;
        }

        static JSStringRef to_jsc_string_copy(const char* val)
        {
            return JSStringCreateWithUTF8CString(val);
        }
    };


    template <>
    struct convert<std::string>
    {
        using type = std::string;

        static type from_jsc_string(JSStringRef val)
        {
            auto size = JSStringGetMaximumUTF8CStringSize(val);

            type ret;
            ret.resize(size);

            JSStringGetUTF8CString(val, &ret.front(), size);

            ret.resize(strlen(ret.c_str()));
            return ret;
        }

        static type from_jsc(JSValueRef val)
        {
            auto str = JSValueToStringCopy(jsc_context, val, nullptr);
            type ret = from_jsc_string(str);

            JSStringRelease(str);

            return ret;
        }

        static JSValueRef to_jsc(const type& val)
        {
            return convert<const char*>::to_jsc(val.c_str());
        }

        static JSStringRef to_jsc_string_copy(const type& val)
        {
            return convert<const char*>::to_jsc_string_copy(val.c_str());
        }
    };

    template<>
    struct convert<bool>
    {
        using type = bool;

        static type from_jsc(JSValueRef value)
        {
            return JSValueToBoolean(jsc_context, value);
        }

        static JSValueRef to_jsc(bool value)
        {
            return JSValueMakeBoolean(jsc_context, value);
        }
    };

    template<typename T>
    struct convert<T, typename std::enable_if<std::is_arithmetic<T>::value>::type>
    {
        using type = T;

        static type from_jsc(JSValueRef value)
        {
            return type(JSValueToNumber(jsc_context, value, nullptr));
        }

        static JSValueRef to_jsc(type value)
        {
            return JSValueMakeNumber(jsc_context, double(value));
        }
    };

    template <>
    struct convert<void>
    {
        using type = void;

        static void from_jsc(JSValueRef) {}
        static JSValueRef to_jsc(type) { return JSValueMakeUndefined(internal::jsc_context); }
    };

    template <>
    struct convert<local>
    {
        using type = local;

        static type from_jsc(JSValueRef value);
        static JSValueRef to_jsc(type value);
    };

    ///////////////////////////////////////////////////////////////////////////

    template <typename T>
    T convert_wrapped_class_from_jsc(JSValueRef value);

    template <typename T>
    JSValueRef convert_wrapped_class_to_jsc(const T& t);

    template <typename T>
    struct convert<T, typename std::enable_if<is_wrapped_class<T>::value>::type>
    {
        using type = T;

        static type from_jsc(JSValueRef value)
        {
            return convert_wrapped_class_from_jsc<type>(value);
        }

        static JSValueRef to_jsc(const T& value)
        {
            return convert_wrapped_class_to_jsc(value);
        }
    };

    ///////////////////////////////////////////////////////////////////////////

    template<typename T>
    struct convert<T&> : convert<T>{};

    template<typename T>
    struct convert<const T&> : convert<T>{};

    ///////////////////////////////////////////////////////////////////////////

    template<typename T>
    typename convert<T>::type from_jsc(JSValueRef value)
    {
        return convert<T>::from_jsc(value);
    }

    template<size_t N>
    JSValueRef to_jsc(const char(&str)[N])
    {
        return convert<char const*>::to_jsc(str);
    }

    template<typename T>
    JSValueRef to_jsc(const T& value)
    {
        return convert<T>::to_jsc(value);
    }

    template<size_t N>
    JSStringRef to_jsc_string_copy(const char(&str)[N])
    {
        return convert<const char*>::to_jsc_string_copy(str);
    }

    inline JSStringRef to_jsc_string_copy(const std::string& str)
    {
        return convert<std::string>::to_jsc_string_copy(str);
    }

    inline JSStringRef to_jsc_string_copy(const char* str)
    {
        return convert<const char*>::to_jsc_string_copy(str);
    }

    template <typename T>
    JSStringRef to_jsc_string_copy(const T& t)
    {
        auto val = to_jsc(t);
        return JSValueToStringCopy(jsc_context, val, nullptr);
    }
}
}
