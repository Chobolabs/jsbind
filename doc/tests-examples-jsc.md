# jsbind tests and examples for JavaScriptCore

> Mac and Linux not yet supported!

## On Windows

1. Get [JSC-Standalone](https://github.com/iboB/jsc-standalone.git)
    * Clone it to `<jsbind-root>/third_party/jsc-standalone`
    * ... or somewhere else and add the configuration option `-DJSBIND_JSC_JSC_STANDALONE_DIR=<your-path>` when you configure the project
2. Configure with CMake *for a 32-bit build*
    * `> cmake path/to/jsbind/repo -DJSBIND_JSC=1 -DJSBIND_ENABLE_TESTING=1 -DJSBIND_BUILD_EXAMPLES=1 -DJSBIND_DEBUGGING=1 -G "Your CMake generator"`
    * *don't forget* `-DJSBIND_JSC_JSC_STANDALONE_DIR=<your-path>` *if you have JSC-Standalone in your own path*
3. Build with an IDE or the command line
4. Follow the instructions in [JSC-Standalone's README](https://github.com/iboB/jsc-standalone/blob/master/README.md#windows) for additional setup to get the required DLLs
5. Run tests with your IDE or `ctest` on the command line
