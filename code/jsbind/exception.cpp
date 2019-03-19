// jsbind
// Copyright (c) 2019 Chobolabs Inc.
// http://www.chobolabs.com/
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// http://opensource.org/licenses/MIT
//
#include "exception.h"

#include <iostream>

namespace jsbind
{

namespace
{

class default_exception_handler : public exception_handler
{
    virtual void on_exception(const char* exception) override
    {
        std::cerr << exception << std::endl;
    }

    virtual void on_engine_error(const char* error) override
    {
        on_exception(error);
    }

} g_default_exception_handler;

exception_handler* g_exception_handler = nullptr;

}

void set_exception_handler(exception_handler* handler)
{
    g_exception_handler = handler;
}

exception_handler* get_exception_handler()
{
    return g_exception_handler;
}

void set_default_exception_handler()
{
    set_exception_handler(&g_default_exception_handler);
}

}