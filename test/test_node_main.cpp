// jsbind
// Copyright (c) 2019 Chobolabs Inc.
// http://www.chobolabs.com/
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// http://opensource.org/licenses/MIT
//
#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

#include <jsbind.h>
#include <testlib.h>

#if !defined(JSBIND_NODE)
#   error "This file is for the jsbind node.js bindings"
#endif

using node::AtExit;

void jsbind_run_tests_node()
{
    jsbind::test::jsbind_init_tests();

    int res = doctest::Context().run();

    jsbind::test::jsbind_deinit_tests();
}

void at_exit(void*)
{
    jsbind::deinitialize();
}

JSBIND_BINDINGS(Tests)
{
    jsbind::function("run", jsbind_run_tests_node);
    AtExit(at_exit);
}

void node_main(v8::Local<v8::Object> exports)
{
    jsbind::v8_initialize_with_global(exports);
}

NODE_MODULE(tests, node_main)