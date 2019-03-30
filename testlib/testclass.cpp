// jsbind
// Copyright (c) 2019 Chobolabs Inc.
// http://www.chobolabs.com/
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// http://opensource.org/licenses/MIT
//
#include "testclass.hpp"
#include <jsbind.hpp>

JSBIND_BINDINGS(testclass)
{
    using namespace jsbind;
    using namespace test;
    class_<testclass>("TestClass")
        .class_function("setStaticData", &testclass::set_static_data)
        ;
}

namespace jsbind
{
namespace test
{

int testclass::set_static_data(const std::string& s, int i, float f, const double& d)
{
    s_s = s;
    s_i = i;
    s_f = f;
    s_d = d;

    return s_i + 10;
}

std::string testclass::s_s = "<?>";
int testclass::s_i = -1;
float testclass::s_f = -0.1f;
double testclass::s_d = -0.01;

}
}
