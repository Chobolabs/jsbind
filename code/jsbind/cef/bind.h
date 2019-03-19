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
    // non template parent
    class class_data
    {
    public:
        CefRefPtr<CefV8Value> m_cef_func;

        static CefRefPtr<CefV8Value> global;
    };
}

template <typename ReturnType, typename... Args>
void function(const char* js_name, ReturnType(*func)(Args...))
{
    CefString cef_name;
    cef_name.FromASCII(js_name);

    auto handler = new internal::cef_handler<ReturnType, Args...>(func);
    auto jsfunc = CefV8Value::CreateFunction(cef_name, handler);

    internal::class_data::global->SetValue(cef_name, jsfunc, V8_PROPERTY_ATTRIBUTE_NONE);
}

template <typename T>
class class_ : private internal::class_data
{
public:
    class_(const char* js_name)
    {
        m_cef_func = CefV8Value::CreateObject(nullptr, nullptr);

        CefString cef_name;
        cef_name.FromASCII(js_name);
        global->SetValue(cef_name, m_cef_func, V8_PROPERTY_ATTRIBUTE_NONE);
    }

    template <typename ReturnType, typename... Args>
    class_& class_function(const char* js_name, ReturnType (*class_func)(Args...))
    {
        CefString cef_name;
        cef_name.FromASCII(js_name);

        auto handler = new internal::cef_handler<ReturnType, Args...>(class_func);
        auto func = CefV8Value::CreateFunction(cef_name, handler);

        m_cef_func->SetValue(cef_name, func, V8_PROPERTY_ATTRIBUTE_NONE);
        return *this;
    }

private:

};

namespace internal
{
    struct value_object_field
    {
        CefString field_name;
        void* pfield;
        void(*from_cef)(CefRefPtr<CefV8Value> value, void* obj, void* pfield);
        CefRefPtr<CefV8Value>(*to_cef)(const void* obj, void* pfield);
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
    static void field_from_cef(CefRefPtr<CefV8Value> value, void* obj, void* pfield)
    {
        auto t = reinterpret_cast<T*>(obj);
        Field field = internal::ptr_cast<Field>(pfield);
        using FieldType = typename internal::function_traits<Field>::return_type;
        t->*field = internal::from_cef<FieldType>(value);
    }

    template <typename Field>
    static CefRefPtr<CefV8Value> field_to_cef(const void* obj, void* pfield)
    {
        auto t = reinterpret_cast<const T*>(obj);
        Field field = internal::ptr_cast<Field>(pfield);
        return internal::to_cef(t->*field);
    }

    template <typename Field>
    value_object& field(const char* js_name, Field field)
    {
        internal::value_object_field f =
        {
            internal::to_cef_string(js_name),
            internal::ptr_cast<Field>(field),
            field_from_cef<Field>,
            field_to_cef<Field>
        };
        fields.emplace_back(std::move(f));
        return *this;
    }

    static std::vector<internal::value_object_field> fields;

#if !defined(NDEBUG)
    static bool is_bound;
#endif

private:
    static void clear_private_data()
    {
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
    T convert_wrapped_class_from_cef(CefRefPtr<CefV8Value> value)
    {
        assert(value_object<T>::is_bound && "casting to an unbound value_type");
        T ret;
        for (auto& field : value_object<T>::fields)
        {
            auto prop = value->GetValue(field.field_name);
            field.from_cef(prop, &ret, field.pfield);
        }
        return ret;
    }

    template <typename T>
    CefRefPtr<CefV8Value> convert_wrapped_class_to_cef(const T& value)
    {
        assert(value_object<T>::is_bound && "casting from an unbound value_type");
        auto ret = CefV8Value::CreateObject(nullptr, nullptr);
        for (auto& field : value_object<T>::fields)
        {
            ret->SetValue(field.field_name, field.to_cef(&value, field.pfield), V8_PROPERTY_ATTRIBUTE_NONE);
        }
        return ret;
    }

}

}
