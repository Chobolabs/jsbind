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
#include <set>
#include <cstdint>

namespace jsbind
{
namespace test
{

class person
{
public:
    person();
    ~person();

    void set_age(float age) { m_age = age; }
    float get_age() const { return m_age; }

    std::string tag;

    static void init_static();
    static const int class_id = 12;
    static int class_custom_data;
    static const std::string& get_class_name();
    static uint32_t get_num_persons();
    static float get_mean_age();

private:
    static std::set<person*> m_all_persons;

    float m_age = 0;
};

}
}