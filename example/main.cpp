// jsbind
// Copyright (c) 2019 Chobolabs Inc.
// http://www.chobolabs.com/
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// http://opensource.org/licenses/MIT
//
#include "jsbind.h"

#if !defined(JSBIND_NODE)

#include <iostream>
#include <string>
#include <fstream>

using namespace std;

int main()
{
    jsbind::initialize();
    jsbind::enter_context();

    string scriptA = "foo = 'global hi from js';";
    string scriptB = "console.log(foo);";

    jsbind::run_script(scriptA.c_str(), "f1");
    jsbind::run_script(scriptB.c_str(), "f2");

    jsbind::exit_context();
    jsbind::deinitialize();

    return 0;
}

#else

#include <node.h>
#include <iostream>

using namespace v8;
using namespace std;

struct ThePipi
{
    static int bar(const std::string& x, float y)
    {
        cout << "Hi, from The Pipi. You gave me " << x << " and " << y << endl;
        return int(floor(y));
    }
};

int hello(int i, const std::string& s)
{
    cout << "Hi, from c++. You gave me " << i << " and " << s << endl;

    jsbind::local::global().set("asd", 666);

    return i + 6;
}

JSBIND_BINDINGS(Example)
{
    jsbind::function("hello", hello);

    jsbind::class_<ThePipi>("pipi")
        .class_function("bar", &ThePipi::bar);
}

void init(Local<Object> exports)
{
    jsbind::v8_initialize_with_global(exports);
}

NODE_MODULE(example, init)

#endif
