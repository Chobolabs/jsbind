// jsbind
// Copyright (c) 2019 Chobolabs Inc.
// http://www.chobolabs.com/
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// http://opensource.org/licenses/MIT
//
#include "deinitializers.h"
#include <vector>

namespace
{
std::vector<jsbind::internal::deinitializer> deinitializers;
}

namespace jsbind
{
namespace internal
{

    void add_deinitializer(deinitializer d)
    {
        deinitializers.push_back(d);
    }

    void run_deinitializers()
    {
        for (auto& d : deinitializers)
        {
            d();
        }

        deinitializers.clear();
    }

}
}