# jsbind

**jsbind** is a C++ :left_right_arrow: JavaScript binding library which supports multitple JavaScript engines and backends.

**!!!NOTE:**

> This repository is still a work in progress. Even though we use the library in production, it's nowhere near open-source friendly (yet). We'll be working to change that in the following weeks. So, until this note is gone, please accept that the version you see here has no documentation and no support.

## Table of contents

- [Features](#features)
- [Motivation](#motivation)
- [Usage](#usage)
  - [Adding jsbind to your project](#adding-jsbind-to-your-project)
  - [Basic usage](#basic-usage)
  - [Reference](#reference)
  - [Integration notes](#integration-notes)
- [Contributing](#contributing)
  - [Building the tests and examples](#building-the-tests-and-examples)
- [Alternatives](#alternatives)
- [Copyright](#copyright)

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
* C++11 compatible

## Motivation

We had a very complex stack for [Mayhem](http://www.playmayhem.com/). It's a game with a C++ engine with some elements of the gameplay and pretty muche the entire metagame logic in JavaScript. The target platforms for playing the game were iOS and Android. We also supported the browser as a target though not for playing the game, but for spectating and watching replays. The development platforms were Windows, macOS and Linux. The game's server ran on a Linux containter. We also had an editor written in JavaScript which could run in a browser or as a local application on a custom Chromium browser (through CEF) with the engine embedded inside.

Eventually we had a core in C++ and JavaScript which we wanted to run on all those platforms. Initially we had the C++/JS bidning layer written in concrete ways for all platforms, but this quickly got out of hand. So we created jsbind. A binding library which lets you support all those platforms with a single codebase.

JS is a very popular language. There is no doubt that other developers want to combine it with C++. If they have similar complex stacks, perhaps it can be of help to them.

## Usage

Add this repository to your project's directory structure either as a submodule or as a copy.

### Adding jsbind to your project

#### CMake

* Set exactly *one* of these CMake options to `TRUE` to describe the desired JS backend. This can be done with the command line or in your `CMakeLists.txt` files:
    * `JSBIND_NODE` for node.js
    * `JSBIND_V8` for pure v8
    * `JSBIND_EMSCRIPTEN` for emscripten
    * `JSBIND_CEF` for CEF
    * `JSBIND_JSC` for JavaScriptCore
* optionally set `JSBIND_JS_BACKEND_LIBS` to the one or more targets which describe the JS backend.
    * This is not needed for emscripten
    * If you do not set this, you will have to use `include_directories` to set the appropriate include directories for jsbind and then link your targets with the appropriate libraries.
* `add_subdirectory(path/to/jsbind/subdir)` to add the jsbind static library target.

### Basic usage

### Reference

A full reference of all functions and classes is available [here](doc/reference.md).

### Integration notes

Here are some backend specific instructions for integrating jsbind:

#### node.js

For node.js the library cannot create the JS engine context, so it can only be initialized with `jsbind::v8_initialize_with_global`. There is a simple example of doing so [the node.js specific test file](test/test_node_main.cpp).

Do not forget to deinitialize jsbind or you may get a crash because of dangling v8 refs when exiting. `node::AtExit` is probably the way to go. Again there is a simple example of using it in [the node.js specific test file](test/test_node_main.cpp).

#### v8

The library can create or be bound to a single v8 context. Multiple contexts are currently not supported.

#### Emscripten

You need to link emscripten "executables" which use jsbind with the `--bind` linker flag.

#### CEF

jsbind can only run on a **render process** in CEF and is bound to a single CEF frame (`iframe`). The frame in CEF is the unit which owns a distinct JS v8 context and they can't be shared.

The library can be bound to a single CEF frame and its context. Multiple contexts are currently not supported, though if you run in multiple processes (as you should) you can have a jsbind instance for each of them.

#### JavaScriptCore

The library can create a single JSC context. Binding to an existing context and creating multiple contexts are currently not supported.

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
