# jsbind

**jsbind** is a C++ :left_right_arrow: JavaScript binding library which supports multitple JavaScript engines and backends.

**!!!NOTE:**

> This repository is still a work in progress. Even though we use the library in production, it's nowhere near open-source friendly (yet). We'll be working to change that in the following weeks. So, until this note is gone, please accept that the version you see here has no documentation and no support.

## Features

Allows the users to have C++ bindings with the same codebase to the following JavaScript backends:

* [node.js](https://nodejs.org/) C++ Addons
* Pure [v8](https://v8.dev/)
* [Emscripten](https://emscripten.org/)
* [CEF](https://bitbucket.org/chromiumembedded/cef)
* [JavaScriptCore](https://trac.webkit.org/wiki/JavaScriptCore)

Supports:

* Executing JS code
* JS object creation
* Calling of JS functions
* Exposing C++ functions to JS
* Seamless integration of C++ built-in types and `std::string`
* Defining custom value types for seamless integration
* Sharing memory between JS ArrayBuffer-s and C++

## Usage

### Adding jsbind to your project

### Basic usage

### Reference

### Integration notes

#### node.js

#### v8

#### Emscripten

#### CEF

jsbind can only run on a **render process** in CEF and is bound to a single CEF frame (`iframe`). The frame in CEF is the unit which owns a distinct JS v8 context and they can't be shared.

#### JavaScriptCore

## Contributing

Contributions are welcome.

### Building the tests and examples

The follwing links have instructions for how to set up your environment to build the tests and examples included in the repo.

* [node.js](doc/tests-examples-node.md)
* [v8](doc/tests-examples-v8.md)
* [Emscripten](doc/tests-examples-em.md)
* [CEF](doc/tests-examples-cef.md)
* [JavaScriptCore](doc/tests-examples-jsc.md)

## Alternatives

jsbind lacks some features compared to other language binding libaries. Notably it doesn't allow users to seamlessly expose C++ classes to the language. The authors believe this prevents the creation of hard to find bugs and unorthodox object lifetimes for C++ objects.

Still if you absolutely need such features, to our knowledge no solution exists which supports that many backends. However if you're satisfied with a single backend (or two since node and v8 often come hand in hand), here is a list of more feature rich single backend libraries:

* [v8pp](https://github.com/pmed/v8pp) - C++ bindings to v8 and node.js

## Copyright

Copyright &copy; 2019 [Chobolabs Inc.](http://www.chobolabs.com/)

This library is distributed under the MIT Software License. See LICENSE.txt for further details or copy [here](http://opensource.org/licenses/MIT).
