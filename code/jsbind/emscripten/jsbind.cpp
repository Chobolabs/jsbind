// jsbind
// Copyright (c) 2019 Chobolabs Inc.
// http://www.chobolabs.com/
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// http://opensource.org/licenses/MIT
//
#include "jsbind/funcs.h"

#include <emscripten.h>

namespace jsbind
{

void initialize()
{
}

void deinitialize()
{
}

void enter_context()
{
}

void exit_context()
{
}

void run_script(const char* text, const char* filename)
{
    emscripten_run_script(text);
}

}