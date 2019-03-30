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

class console
{
public:
    virtual ~console() {}

    enum msg_type
    {
        msg_log,
        msg_info,
        msg_warn,
        msg_error,
        msg_debug,
        msg_assert,
    };
    virtual void msg(msg_type type, const char* msg) = 0;
};

extern void set_console(console* c);
extern console* get_console();
extern void set_default_console();

}

