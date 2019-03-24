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

template <typename ReturnType, typename... Args>
void function(const char* js_name, ReturnType(*func)(Args...));

template <typename T>
class class_ : private internal::class_data
{
public:
    class_(const char* js_name);

    template <typename ReturnType, typename... Args>
    class_& class_function(const char* js_name, ReturnType (*class_func)(Args...));
};

template <typename T>
class value_object
{
public:
    value_object(const char*);

    template <typename Field>
    value_object& field(const char* js_name, Field field);
};

}
