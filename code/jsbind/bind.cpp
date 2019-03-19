// jsbind
// Copyright (c) 2019 Chobolabs Inc.
// http://www.chobolabs.com/
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// http://opensource.org/licenses/MIT
//
#if !defined(JSBIND_EMSCRIPTEN)
#include "bind.h"

namespace jsbind
{
namespace internal
{
    std::deque<binding_initializer*>& binding_initializer::instances()
    {
        static std::deque<binding_initializer*> i;
        return i;
    }

    binding_initializer::binding_initializer()
    {
        instances().push_back(this);
    }

    void initialize_bindings()
    {
        for (auto b : binding_initializer::instances())
        {
            b->initialize();
        }
    }
}
}

#endif
