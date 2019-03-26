// jsbind
// Copyright (c) 2019 Chobolabs Inc.
// http://www.chobolabs.com/
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// http://opensource.org/licenses/MIT
//
#include "person.h"
#include <jsbind.h>

JSBIND_BINDINGS(person)
{
    using namespace jsbind;
    using jsbind::test::person;

    class_<person>("Person")
        .class_function("initStatic", &person::init_static)
        .class_function("getClassName", &person::get_class_name)
        .class_function("getNumPersons", &person::get_num_persons)
        .class_function("getMeanAge", &person::get_mean_age)
    ;
}

using namespace std;

namespace jsbind
{
namespace test
{

int person::class_custom_data;
set<person*> person::m_all_persons;

static std::string pclass = "<?>";

void person::init_static()
{
    pclass = "Person-class";
}

const string& person::get_class_name() { return pclass; }

uint32_t person::get_num_persons() { return uint32_t(m_all_persons.size()); }

float person::get_mean_age()
{
    float sum = 0;
    for (auto p : m_all_persons)
    {
        sum += p->get_age();
    }

    return sum / get_num_persons();
}

person::person()
{
    m_all_persons.insert(this);
}

person::~person()
{
    m_all_persons.erase(this);
}

}
}
