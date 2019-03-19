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
        using from_type = std::string;
        using to_type = v8::Local<v8::String>;

        static from_type from_v8(v8::Local<v8::Value> val)
        {
            const v8::String::Utf8Value str(val);
            return from_type(*str, str.length());
        }

        static to_type to_v8(const from_type& val)
        {
            return v8::String::NewFromUtf8(isolate, val.c_str(),
                v8::String::kNormalString, int(val.length()));
        }
    };


    template <>
    struct convert<const char*>
    {
        using from_type = const char*;
        using to_type = v8::Local<v8::String>;

        // raw pointer converion not supported
        // it could be supported but emscripten doesn't do it, so we'll follow
        static from_type from_v8(v8::Local<v8::Value> val) = delete;

        static to_type to_v8(const char* val)
        {
            return v8::String::NewFromUtf8(isolate, val);
        }
    };

    template<>
    struct convert<bool>
    {
        using from_type = bool;
        using to_type = v8::Local<v8::Boolean>;

        static from_type from_v8(v8::Local<v8::Value> value)
        {
            return value->ToBoolean()->Value();
        }

        static to_type to_v8(bool value)
        {
            return v8::Boolean::New(isolate, value);
        }
    };

    template<typename T>
    struct convert<T, typename std::enable_if<std::is_integral<T>::value>::type>
    {
        using from_type = T;
        using to_type = v8::Local<v8::Number>;

        enum { bits = sizeof(T) * CHAR_BIT, is_signed = std::is_signed<T>::value };

        static from_type from_v8(v8::Local<v8::Value> value)
        {
            if (bits <= 32)
            {
                if (is_signed)
                {
                    return static_cast<T>(value->Int32Value());
                }
                else
                {
                    return static_cast<T>(value->Uint32Value());
                }
            }
            else
            {
                return static_cast<T>(value->IntegerValue());
            }
        }

        static to_type to_v8(from_type value)
        {
            if (bits <= 32)
            {
                if (is_signed)
                {
                    return v8::Integer::New(isolate, static_cast<int32_t>(value));
                }
                else
                {
                    return v8::Integer::NewFromUnsigned(isolate, static_cast<uint32_t>(value));
                }
            }
            else
            {
                // check value < (1<<57) to fit in double?
                return v8::Number::New(isolate, static_cast<double>(value));
            }
        }
    };

    template<typename T>
    struct convert<T, typename std::enable_if<std::is_floating_point<T>::value>::type>
    {
        using from_type = T;
        using to_type = v8::Local<v8::Number>;

        static from_type from_v8(v8::Local<v8::Value> value)
        {
            return static_cast<T>(value->NumberValue());
        }

        static to_type to_v8(T value)
        {
            return v8::Number::New(isolate, value);
        }
    };

    template <>
    struct convert<void>
    {
        using from_type = void;
        using to_type = v8::Local<v8::Primitive>;

        static from_type from_v8(v8::Local<v8::Value>) {}
        static to_type to_v8() { return v8::Undefined(isolate); }
    };

    template <>
    struct convert<local>
    {
        using from_type = local;
        using to_type = v8::Local<v8::Value>;

        static from_type from_v8(v8::Local<v8::Value>);
        static to_type to_v8(const from_type&);
    };

    template<typename T>
    struct convert<T&> : convert<T>{};

    template<typename T>
    struct convert<const T&> : convert<T>{};

    ///////////////////////////////////////////////////////////////////////////

    template <typename T>
    T convert_wrapped_class_from_v8(v8::Local<v8::Value> value);

    template <typename T>
    v8::Local<v8::Object> convert_wrapped_class_to_v8(const T& t);

    template <typename T>
    struct convert<T, typename std::enable_if<is_wrapped_class<T>::value>::type>
    {
        using from_type = T;
        using to_type = v8::Local<v8::Object>;

        static from_type from_v8(v8::Local<v8::Value> value)
        {
            return convert_wrapped_class_from_v8<from_type>(value);
        }

        static to_type to_v8(const from_type& value)
        {
            return convert_wrapped_class_to_v8(value);
        }
    };

    ///////////////////////////////////////////////////////////////////////////

    template<typename T>
    typename convert<T>::from_type from_v8(v8::Local<v8::Value> value)
    {
        return convert<T>::from_v8(value);
    }

    template<size_t N>
    v8::Local<v8::String> to_v8(const char (&str)[N])
    {
        return convert<const char*>::to_v8(str);
    }

    template<typename T>
    typename convert<T>::to_type to_v8(const T& value)
    {
        return convert<T>::to_v8(value);
    }
}
}
