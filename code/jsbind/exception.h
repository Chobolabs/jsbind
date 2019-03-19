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

class exception_handler
{
public:
    virtual ~exception_handler() {}
    virtual void on_exception(const char* exception) = 0;
    virtual void on_engine_error(const char* error) = 0;
};

extern void set_exception_handler(exception_handler* handler);
extern exception_handler* get_exception_handler();
extern void set_default_exception_handler();

}
