#define DOCTEST_CONFIG_IMPLEMENT
#include <doctest/doctest.h>

#include <jsbind/funcs.h>
#include <testlib.h>

int main(int argc, char* argv[])
{
    jsbind::initialize();
    jsbind::enter_context();
    jsbind_init_tests();

    doctest::Context context(argc, argv);
    int res = context.run();

    jsbind_deinit_tests();
    jsbind::exit_context();
    jsbind::deinitialize();

    puts("\n"); // long story: emcc requires this

    return res;
}
