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
        v8::Local<v8::FunctionTemplate> m_v8func;

        static v8::Local<v8::ObjectTemplate>* global;
    };
}

template <typename ReturnType, typename... Args>
void function(const char* js_name, ReturnType(*func)(Args...))
{
    auto ft = v8::FunctionTemplate::New(internal::isolate,
        internal::call_class_function_from_v8<ReturnType, Args...>,
        v8::External::New(internal::isolate, reinterpret_cast<void*>(func)));

    auto& g = *internal::class_data::global;
    g->Set(internal::isolate, js_name, ft);
}

template <typename T>
class class_ : private internal::class_data
{
public:
    class_(const char* js_name)
    {
        m_v8func = v8::FunctionTemplate::New(internal::isolate);
        auto& g = *global;
        g->Set(internal::isolate, js_name, m_v8func);
    }

    template <typename ReturnType, typename... Args>
    class_& class_function(const char* js_name, ReturnType (*class_func)(Args...))
    {
        auto ft = v8::FunctionTemplate::New(internal::isolate,
            internal::call_class_function_from_v8<ReturnType, Args...>,
            v8::External::New(internal::isolate, reinterpret_cast<void*>(class_func)));
        m_v8func->Set(internal::isolate, js_name, ft);
        return *this;
    }

private:

};

namespace internal
{
    struct value_object_field
    {
        using name_type = v8::CopyablePersistentTraits<v8::String>::CopyablePersistent;
        name_type field_name;
        void* pfield;
        void(*from_v8)(v8::Local<v8::Value> value, void* obj, void* pfield);
        v8::Local<v8::Value>(*to_v8)(const void* obj, void* pfield);
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
    static void field_from_v8(v8::Local<v8::Value> value, void* obj, void* pfield)
    {
        auto t = reinterpret_cast<T*>(obj);
        Field field = internal::ptr_cast<Field>(pfield);
        using FieldType = typename internal::function_traits<Field>::return_type;
        t->*field = internal::from_v8<FieldType>(value);
    }

    template <typename Field>
    static v8::Local<v8::Value> field_to_v8(const void* obj, void* pfield)
    {
        auto t = reinterpret_cast<const T*>(obj);
        Field field = internal::ptr_cast<Field>(pfield);
        return internal::to_v8(t->*field);
    }

    template <typename Field>
    value_object& field(const char* js_name, Field field)
    {
        internal::value_object_field f = {
            internal::value_object_field::name_type(internal::isolate, internal::to_v8(js_name)),
            internal::ptr_cast<Field>(field),
            field_from_v8<Field>,
            field_to_v8<Field>
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
    T convert_wrapped_class_from_v8(v8::Local<v8::Value> value)
    {
        assert(value_object<T>::is_bound && "casting to an unbound value_type");
        auto obj = v8::Object::Cast(*value);
        T ret;
        for (auto& field : value_object<T>::fields)
        {
            auto name = *reinterpret_cast<v8::Local<v8::String>*>(&field.field_name);
            auto prop = obj->Get(name);
            field.from_v8(prop, &ret, field.pfield);
        }
        return ret;
    }

    template <typename T>
    v8::Local<v8::Object> convert_wrapped_class_to_v8(const T& value)
    {
        assert(value_object<T>::is_bound && "casting from an unbound value_type");
        auto ret = v8::Object::New(internal::isolate);
        for (auto& field : value_object<T>::fields)
        {
            auto name = *reinterpret_cast<v8::Local<v8::String>*>(&field.field_name);
            ret->Set(name, field.to_v8(&value, field.pfield));
        }
        return ret;
    }

}

}