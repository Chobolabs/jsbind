# jsbind tests and examples node.js

1. You need a node.js instalation and node-gyp.
    * On Ubuntu they can come from apt. `sudo apt install node-gyp` should be enough.
    * If you want the latest node.js version on Ubuntu or you're on  another platform, use the suggested way of installing node.js. For example the [packages from their website](https://nodejs.org/en/download/). Then install node-gyp globally with `npm install -g node-gyp`
2. Install node headers and libraries
    * `$ node-gyp install`
3. Configure with CMake
    * `$ cmake path/to/jsbind/repo -DJSBIND_NODE=1 -DJSBIND_ENABLE_TESTING=1 -DJSBIND_BUILD_EXAMPLES=1 -DJSBIND_DEBUGGING=1 -G "Your CMake generator"`
    * Note that on Windows the CMake generator must be the appropriate visual studio version: `"Visual Studio 15 2017"` or `"Visual Studio 15 2017 Win64"` for node.js 10+
4. Build with an IDE or the command line
5. Run tests with your IDE or `ctest` on the command line
