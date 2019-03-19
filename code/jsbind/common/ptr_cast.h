// jsbind
// Copyright (c) 2019 Chobolabs Inc.
// http://www.chobolabs.com/
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// http://opensource.org/licenses/MIT
//
#pragma once

namespace jsbind
{
namespace internal
{

    template <typename T>
    union ptr_cast
    {
        static_assert(sizeof(T) <= sizeof(void*), "size of object in ptr_cast must not be greater than ptr size");
        T value;
        void* ptr;

        explicit ptr_cast(void* p) : ptr(p) {}

        explicit ptr_cast(T v) : value(v) {}

        operator void*() const { return ptr; }
        operator T() const { return value; }
    };

}
}
