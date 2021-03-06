// jsbind
// Copyright (c) 2019 Chobolabs Inc.
// http://www.chobolabs.com/
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// http://opensource.org/licenses/MIT
//
#include "pods.hpp"
#include <jsbind.hpp>

JSBIND_BINDINGS(pods)
{
    using namespace jsbind;
    using namespace jsbind::test;

    value_object<vec>("Vec")
        .field("x", &vec::x)
        .field("y", &vec::y)
        ;

    value_object<mec>("Mec")
        .field("i", &mec::i)
        .field("f", &mec::f)
        ;

    value_object<sec>("Sec")
        .field("name", &sec::name)
        .field("age", &sec::age)
        ;

    function("getStoredVec", &get_stored_vec);
    function("storeVec", &store_vec);
    function("getStoredMec", &get_stored_mec);
    function("storeMec", &store_mec);
    function("getStoredSec", &get_stored_sec);
    function("storeSec", &store_sec);
    function("getStoredNec", &get_stored_nec);
    function("storeNec", &store_nec);
}

namespace
{
jsbind::test::vec the_vec;
jsbind::test::mec the_mec;
jsbind::test::sec the_sec;
jsbind::test::nec the_nec;
}

namespace jsbind
{
namespace test
{

void store_vec(const vec& v)
{
    the_vec = v;
}

vec get_stored_vec()
{
    return the_vec;
}

void store_mec(const mec& v)
{
    the_mec = v;
}

mec get_stored_mec()
{
    return the_mec;
}

void store_sec(const sec& v)
{
    the_sec = v;
}

sec get_stored_sec()
{
    return the_sec;
}

void store_nec(const nec& n)
{
    the_nec = n;
}

nec get_stored_nec()
{
    return the_nec;
}

}
}
