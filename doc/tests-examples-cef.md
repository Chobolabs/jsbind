# jsbind tests and examples for CEF

1. Get [CEF-CMake](https://github.com/iboB/cef-cmake)
    * Clone it to `<jsbind-root>/third_party/cef-cmake`
    * ... or somewhere else and add the configuration option `-DJSBIND_CEF_CEF_CMAKE_DIR=<your-path>` when you configure the project
2. Configure with CMake
    * `$ cmake path/to/jsbind/repo -DJSBIND_CEF=1 -DJSBIND_ENABLE_TESTING=1 -DJSBIND_BUILD_EXAMPLES=1 -DJSBIND_DEBUGGING=1 -G "Your CMake generator"`
    * *don't forget* `-DJSBIND_CEF_CEF_CMAKE_DIR=<your-path>` *if you have CEF-CMake in your own path*
4. Build with an IDE or the command line
5. Run tests with your IDE or `ctest` on the command line
