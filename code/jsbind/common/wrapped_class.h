// jsbind
// Copyright (c) 2019 Chobolabs Inc.
// http://www.chobolabs.com/
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// http://opensource.org/licenses/MIT
//
#pragma once

#include <type_traits>
#include <string>

#include "jsbind/value_fwd.h"

namespace jsbind
{
namespace internal
{

    template <typename T, typename Enable = void>
    struct is_wrapped_class;

    template <typename T>
    struct is_wrapped_class<T> : std::is_class<T> {};

    template <>
    struct is_wrapped_class<std::string> : std::false_type {};

    template <>
    struct is_wrapped_class<local> : std::false_type {};

}
}
