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
    typedef void(*deinitializer)();

    extern void add_deinitializer(deinitializer d);
    extern void run_deinitializers();
}
}
