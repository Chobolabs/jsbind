// jsbind
// Copyright (c) 2019 Chobolabs Inc.
// http://www.chobolabs.com/
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// http://opensource.org/licenses/MIT
//
#pragma once

#include <string>

namespace test
{

class testclass
{
public:
    // returns i + 10
    static int set_static_data(const std::string& s, int i, float f, const double& d);

    static const std::string& get_static_string() { return s_s; }
    static int get_static_int() { return s_i; }
    static float get_static_float() { return s_f; }
    static const double& get_static_double() { return s_d; }

    static std::string s_s;
    static int s_i;
    static float s_f;
    static double s_d;
};

}
