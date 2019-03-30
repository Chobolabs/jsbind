// jsbind
// Copyright (c) 2019 Chobolabs Inc.
// http://www.chobolabs.com/
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// http://opensource.org/licenses/MIT
//
#define DOCTEST_CONFIG_NO_POSIX_SIGNALS
#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

#include <jsbind/funcs.hpp>
#include <testlib.hpp>

int main(int argc, char* argv[])
{
    jsbind::initialize();
    jsbind::enter_context();
    jsbind::test::jsbind_init_tests();

    doctest::Context context(argc, argv);
    int res = context.run();

    jsbind::test::jsbind_deinit_tests();
    jsbind::exit_context();
    jsbind::deinitialize();

    return res;
}
