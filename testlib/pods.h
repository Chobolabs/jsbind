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

struct vec
{
    float x, y;
};

void store_vec(const vec& v);
vec get_stored_vec();

struct mec
{
    int i;
    float f;
};

void store_mec(const mec& m);
mec get_stored_mec();

struct sec
{
    std::string name;
    int age;
};

void store_sec(const sec& m);
sec get_stored_sec();

struct nec
{
    int foo;
};

void store_nec(const nec& m);
nec get_stored_nec();

}