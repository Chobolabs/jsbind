// jsbind
// Copyright (c) 2019 Chobolabs Inc.
// http://www.chobolabs.com/
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// http://opensource.org/licenses/MIT
//
#pragma once

#include "value.h"
#include "jsbind/common/ptr_cast.h"
#include "jsbind/common/deinitializers.h"

namespace jsbind
{

namespace internal
{
    class class_data
    {
    public:
        JSObjectRef m_jsc_func;

        static JSObjectRef global;

        static JSClassRef function_class;
    };
}

template <typename ReturnType, typename... Args>
void function(const char* js_name, ReturnType(*func)(Args...))
{
    auto func_data = new internal::call_class_function_from_jsc<ReturnType, Args...>(func);
    auto js_func = JSObjectMake(internal::jsc_context, internal::class_data::function_class, func_data);

    auto name = internal::to_jsc_string_copy(js_name);
    JSObjectSetProperty(internal::jsc_context, internal::class_data::global, name, js_func, kJSPropertyAttributeNone, nullptr);
    JSStringRelease(name);
}

template <typename T>
class class_ : private internal::class_data
{
public:
    class_(const char* js_name)
    {
        auto name = internal::to_jsc_string_copy(js_name);
        //JSObjectMakeFunction(internal::jsc_context, name, 0, nullptr, nullptr, nullptr, 0, nullptr);
        m_jsc_func = JSObjectMake(internal::jsc_context, nullptr, nullptr);
        JSValueProtect(internal::jsc_context, m_jsc_func);

        JSObjectSetProperty(internal::jsc_context, global, name, m_jsc_func, kJSPropertyAttributeNone, nullptr);

        JSStringRelease(name);
    }

    ~class_()
    {
        JSValueUnprotect(internal::jsc_context, m_jsc_func);
    }

    template <typename ReturnType, typename... Args>
    class_& class_function(const char* js_name, ReturnType(*class_func)(Args...))
    {
        auto func_data = new internal::call_class_function_from_jsc<ReturnType, Args...>(class_func);
        auto js_func = JSObjectMake(internal::jsc_context, function_class, func_data);

        auto name = internal::to_jsc_string_copy(js_name);
        JSObjectSetProperty(internal::jsc_context, m_jsc_func, name, js_func, kJSPropertyAttributeNone, nullptr);
        JSStringRelease(name);

        return *this;
    }

private:

};

namespace internal
{
    struct value_object_field
    {
        JSStringRef field_name;
        void* pfield;
        void(*from_jsc)(JSValueRef value, void* obj, void* pfield);
        JSValueRef(*to_jsc)(const void* obj, void* pfield);
    };
}

template <typename T>
class value_object : private internal::class_data
{
public:
    value_object(const char*)
    {
        internal::add_deinitializer(value_object<T>::clear_private_data);
#if !defined(NDEBUG)
        assert(!is_bound && "Multiple exposes of value_object");
        is_bound = true;
#endif
    }

    template <typename Field>
    static void field_from_jsc(JSValueRef value, void* obj, void* pfield)
    {
        auto t = reinterpret_cast<T*>(obj);
        Field field = internal::ptr_cast<Field>(pfield);
        using FieldType = typename internal::function_traits<Field>::return_type;
        t->*field = internal::from_jsc<FieldType>(value);
    }

    template <typename Field>
    static JSValueRef field_to_jsc(const void* obj, void* pfield)
    {
        auto t = reinterpret_cast<const T*>(obj);
        Field field = internal::ptr_cast<Field>(pfield);
        return internal::to_jsc(t->*field);
    }

    template <typename Field>
    value_object& field(const char* js_name, Field field)
    {
        internal::value_object_field f = {
            internal::to_jsc_string_copy(js_name),
            internal::ptr_cast<Field>(field),
            field_from_jsc<Field>,
            field_to_jsc<Field>
        };
        fields.push_back(f);
        return *this;
    }

    static std::vector<internal::value_object_field> fields;

#if !defined(NDEBUG)
    static bool is_bound;
#endif

private:
    static void clear_private_data()
    {
        for (auto& field : fields)
        {
            JSStringRelease(field.field_name);
        }

        fields.clear();
#if !defined(NDEBUG)
        is_bound = false;
#endif
    }
};

template <typename T>
std::vector<internal::value_object_field> value_object<T>::fields;

#if !defined(NDEBUG)
template <typename T>
bool value_object<T>::is_bound;
#endif

namespace internal
{

    template <typename T>
    T convert_wrapped_class_from_jsc(JSValueRef value)
    {
        assert(value_object<T>::is_bound && "casting to an unbound value_type");
        auto obj = JSValueToObject(internal::jsc_context, value, nullptr);
        T ret;
        for (auto& field : value_object<T>::fields)
        {
            auto prop = JSObjectGetProperty(internal::jsc_context, obj, field.field_name, nullptr);
            field.from_jsc(prop, &ret, field.pfield);
        }
        return ret;
    }

    template <typename T>
    JSValueRef convert_wrapped_class_to_jsc(const T& value)
    {
        assert(value_object<T>::is_bound && "casting from an unbound value_type");
        JSObjectRef ret = JSObjectMake(internal::jsc_context, nullptr, nullptr);
        for (auto& field : value_object<T>::fields)
        {
            JSObjectSetProperty(internal::jsc_context, ret, field.field_name,
                field.to_jsc(&value, field.pfield),
                kJSPropertyAttributeNone, nullptr);

        }
        return ret;
    }

}

}

