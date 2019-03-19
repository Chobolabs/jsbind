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
    // with c++14 this can be replaced with std::integer_sequence
    template <size_t... I>
    struct index_sequence {};

    template <size_t N, size_t ...I>
    struct make_index_sequence : make_index_sequence<N - 1, N - 1, I...> {};

    template <size_t ...I>
    struct make_index_sequence<0, I...> : index_sequence<I...>{};
}
}
