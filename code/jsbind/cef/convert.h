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

    template <>
    struct convert<std::string>
    {
        using type = std::string;

        static type from_cef(CefRefPtr<CefV8Value> val)
        {
            const auto str = val->GetStringValue();
            return str.ToString();
        }

        static CefRefPtr<CefV8Value> to_cef(const type& val)
        {
            CefString str;
            str.FromString(val);
            return CefV8Value::CreateString(str);
        }
    };


    template <>
    struct convert<const char*>
    {
        using type = const char*;

        // raw pointer converion not supported
        // it could be supported but emscripten doesn't do it, so we'll follow
        static type from_cef(CefRefPtr<CefV8Value> val) = delete;

        static CefRefPtr<CefV8Value> to_cef(const char* val)
        {
            CefString str;
            str.FromASCII(val);
            return CefV8Value::CreateString(str);
        }
    };

    template<>
    struct convert<bool>
    {
        using type = bool;

        static type from_cef(CefRefPtr<CefV8Value> value)
        {
            return value->GetBoolValue();
        }

        static CefRefPtr<CefV8Value> to_cef(bool value)
        {
            return CefV8Value::CreateBool(value);
        }
    };

    template<typename T>
    struct convert<T, typename std::enable_if<std::is_integral<T>::value>::type>
    {
        using type = T;

        enum { is_signed = std::is_signed<T>::value };

        static type from_cef(CefRefPtr<CefV8Value> value)
        {
            // temporarily commented out since cef returns 0 on those if the underlying
            // js number isn't an integer value

            //if (is_signed)
            //{
            //    return static_cast<T>(value->GetIntValue());
            //}
            //else
            //{
            //    return static_cast<T>(value->GetUIntValue());
            //}

            // instead we'll fall back to this solution until we manually patch
            // CefV8ValueImpl ::GetIntValue ::GetUIntValue in v8_impl.cc

            return type(value->GetDoubleValue());
        }

        static CefRefPtr<CefV8Value> to_cef(type value)
        {
            if (is_signed)
            {
                return CefV8Value::CreateInt(static_cast<int32_t>(value));
            }
            else
            {
                return CefV8Value::CreateUInt(static_cast<int32_t>(value));
            }
        }
    };

    template<typename T>
    struct convert<T, typename std::enable_if<std::is_floating_point<T>::value>::type>
    {
        using type = T;

        static type from_cef(CefRefPtr<CefV8Value> value)
        {
            return static_cast<T>(value->GetDoubleValue());
        }

        static CefRefPtr<CefV8Value> to_cef(T value)
        {
            return CefV8Value::CreateDouble(value);
        }
    };

    template <>
    struct convert<void>
    {
        using type = void;

        static type from_cef(CefRefPtr<CefV8Value>) {}
        static CefRefPtr<CefV8Value> to_cef() { return CefV8Value::CreateUndefined(); }
    };

    template <>
    struct convert<local>
    {
        using type = local;

        static type from_cef(CefRefPtr<CefV8Value>);
        static CefRefPtr<CefV8Value> to_cef(const type&);
    };

    template<typename T>
    struct convert<T&> : convert<T>{};

    template<typename T>
    struct convert<const T&> : convert<T>{};

    ///////////////////////////////////////////////////////////////////////////

    template <typename T>
    T convert_wrapped_class_from_cef(CefRefPtr<CefV8Value> value);

    template <typename T>
    CefRefPtr<CefV8Value> convert_wrapped_class_to_cef(const T& t);

    template <typename T>
    struct convert<T, typename std::enable_if<is_wrapped_class<T>::value>::type>
    {
        using type = T;

        static type from_cef(CefRefPtr<CefV8Value> value)
        {
            return convert_wrapped_class_from_cef<type>(value);
        }

        static CefRefPtr<CefV8Value> to_cef(const type& value)
        {
            return convert_wrapped_class_to_cef(value);
        }
    };

    ///////////////////////////////////////////////////////////////////////////

    template<typename T>
    typename convert<T>::type from_cef(CefRefPtr<CefV8Value> value)
    {
        return convert<T>::from_cef(value);
    }

    template<size_t N>
    CefRefPtr<CefV8Value> to_cef(const char (&str)[N])
    {
        return convert<const char*>::to_cef(str);
    }

    template<typename T>
    CefRefPtr<CefV8Value> to_cef(const T& value)
    {
        return convert<T>::to_cef(value);
    }

    ////////////////////////////////////////////////////////////////////////////

    template<size_t N>
    CefString to_cef_string(const char(&str)[N])
    {
        return CefString(str);
    }

    inline CefString to_cef_string(const std::string& str)
    {
        return CefString(str);
    }

    inline CefString to_cef_string(const char* str)
    {
        return CefString(str);
    }

    template <typename T>
    CefString to_cef_string(const T& t)
    {
        auto val = to_cef(t);
        return val->GetStringValue();
    }
}
}
