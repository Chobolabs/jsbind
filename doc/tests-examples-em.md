# jsbind tests and examples for emscripten

1. Install and activate emscripten using [the instructions on emscripten's website](https://emscripten.org/docs/getting_started/downloads.html)
    * On Windows also install a make tool. For example from [mingw-w64](https://mingw-w64.org/doku.php)
2. Configure with the emscripten CMake toolchain file
    * `$ cmake path/to/jsbind/repo -DCMAKE_TOOLCHAIN_FILE=path/to/emscripten/cmake/Modules/Platform/Emscripten.cmake -DJSBIND_EMSCRIPTEN=1 -DJSBIND_ENABLE_TESTING=1 -DJSBIND_BUILD_EXAMPLES=1 -DJSBIND_DEBUGGING=1 -G "Unix Makefiles"`
3. Build with `make` or `mingw32-make`
4. Run tests with `ctest`
