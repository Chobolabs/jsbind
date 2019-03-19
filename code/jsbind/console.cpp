// jsbind
// Copyright (c) 2019 Chobolabs Inc.
// http://www.chobolabs.com/
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// http://opensource.org/licenses/MIT
//
#include "console.h"

#include <iostream>
#include <cassert>
using namespace std;

namespace jsbind
{

namespace
{
class default_console : public console
{
public:
    virtual void msg(console::msg_type type, const char* msg) override
    {
        switch (type)
        {
        case msg_log:
        case msg_info:
        case msg_warn:
            cout << msg << endl;
            break;
        case msg_debug:
#if !defined(NDEBUG)
            cout << msg << endl;
#endif
            break;
        case msg_assert:
            assert(false);
        case msg_error:
            cerr << msg << endl;
        }
    }
} g_default_console;

console* g_console = nullptr;

}

void set_console(console* c)
{
    g_console = c;
}

console* get_console()
{
    return g_console;
}

void set_default_console()
{
    set_console(&g_default_console);
}

}
