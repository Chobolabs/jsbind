// jsbind
// Copyright (c) 2019 Chobolabs Inc.
// http://www.chobolabs.com/
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// http://opensource.org/licenses/MIT
//
#define DOCTEST_CONFIG_NO_SHORT_MACRO_NAMES
#include "doctest/doctest.h"

#include <jsbind/shared_memory_extension.h>

using namespace jsbind;

DOCTEST_TEST_SUITE("shared memory extension");

DOCTEST_TEST_CASE("creation")
{
    scope s;

    uint8_array ar(10);

    auto data = ar.get_buffer();

    for (int i = 0; i < 10; ++i)
    {
        data[i] = i + 7;
    }

    local buf = ar.get_persistent().to_local();

    DOCTEST_CHECK(buf.typeOf().as<std::string>() == "object");

    auto len = buf["length"];
    DOCTEST_CHECK(len.as<int32_t>() == 10);

    for (int i = 0; i < 5; ++i)
    {
        DOCTEST_CHECK(buf[i].as<int32_t>() == i + 7);
    }

    auto moved(std::move(ar));

    DOCTEST_CHECK(!moved.get_persistent().is_empty());
    DOCTEST_CHECK(ar.get_persistent().is_empty());
    DOCTEST_CHECK(moved.get_buffer() == data);
}
