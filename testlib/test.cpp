// jsbind
// Copyright (c) 2019 Chobolabs Inc.
// http://www.chobolabs.com/
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// http://opensource.org/licenses/MIT
//
#include "jsbind.h"
#include "jsbind/console.h"
#include "jsbind/exception.h"

#include "person.h"
#include "testclass.h"
#include "pods.h"

#include <iostream>
#include <cstdint>
#include <cmath>

#define DOCTEST_CONFIG_NO_SHORT_MACRO_NAMES
#include "doctest/doctest.h"

using namespace jsbind;
using doctest::Approx;

class test_exception_handler : public jsbind::exception_handler
{
    virtual void on_exception(const char* text) override
    {
        ++m_caught;
    }

    virtual void on_engine_error(const char* error) override
    {
        ++m_caught;
    }

    int m_caught = 0;

public:

    int get_num_caught()
    {
        int num = m_caught;
        m_caught = 0;
        return num;
    }
};

test_exception_handler* test_handler = nullptr;

DOCTEST_TEST_CASE("local")
{
    scope s;

    // constructs
    auto undef = local::undefined();
    DOCTEST_CHECK(undef.isUndefined());

    auto null = local::null();
    DOCTEST_CHECK(null.isNull());

    local fls(false);
    DOCTEST_CHECK(fls.isFalse());

    local tr(true);
    DOCTEST_CHECK(tr.isTrue());

    local str("strvalue");
    DOCTEST_CHECK(str.as<std::string>() == "strvalue");

    local integer(23);
    DOCTEST_CHECK(integer.as<int8_t>() == 23);
    DOCTEST_CHECK(integer.as<int16_t>() == 23);
    DOCTEST_CHECK(integer.as<int32_t>() == 23);

    local single(3.14f);
    DOCTEST_CHECK(Approx(single.as<float>()) == 3.14f);
    DOCTEST_CHECK(Approx(single.as<double>()) == 3.14);
    DOCTEST_CHECK(single.as<int8_t>() == 3);

    local dbl(2.71);
    DOCTEST_CHECK(Approx(dbl.as<float>()) == 2.71f);
    DOCTEST_CHECK(Approx(dbl.as<double>()) == 2.71);
    DOCTEST_CHECK(dbl.as<int32_t>() == 2);

    // equals
    local integer2(42);
    DOCTEST_CHECK(integer2.as<int32_t>() == 42);

    integer2 = integer;
    DOCTEST_CHECK(integer2.as<int32_t>() == 23);

    // typeofs
    DOCTEST_CHECK(undef.typeOf().as<std::string>() == "undefined");
    DOCTEST_CHECK(null.typeOf().as<std::string>() == "object");
    DOCTEST_CHECK(fls.typeOf().as<std::string>() == "boolean");
    DOCTEST_CHECK(tr.typeOf().as<std::string>() == "boolean");
    DOCTEST_CHECK(str.typeOf().as<std::string>() == "string");
    DOCTEST_CHECK(integer.typeOf().as<std::string>() == "number");

    // obj and properties
    auto obj = local::object();
    DOCTEST_CHECK(obj.typeOf().as<std::string>() == "object");
    DOCTEST_CHECK(!obj.hasOwnProperty("prop0"));
    DOCTEST_CHECK(!obj.hasOwnProperty("prop1"));

    auto prop = obj["prop0"];
    DOCTEST_CHECK(prop.isUndefined());

    obj.set("prop0", tr);
    DOCTEST_CHECK(obj.hasOwnProperty("prop0"));
    prop = obj["prop0"];
    DOCTEST_CHECK(prop.isTrue());

    obj.set("prop1", 51);
    DOCTEST_CHECK(obj.hasOwnProperty("prop0"));
    prop = obj["prop1"];
    DOCTEST_CHECK(prop.typeOf().as<std::string>() == "number");
    DOCTEST_CHECK(prop.as<int32_t>() == 51);

    obj.set("propInt", integer);
    prop = obj["propInt"];
    DOCTEST_CHECK(prop.typeOf().as<std::string>() == "number");
    DOCTEST_CHECK(prop.as<int32_t>() == 23);

    // array
    auto ar = local::array();
    DOCTEST_CHECK(ar.typeOf().as<std::string>() == "object");
    DOCTEST_CHECK(ar.hasOwnProperty("length"));

    auto len = ar["length"];
    DOCTEST_CHECK(len.as<int32_t>() == 0);

    ar.set("length", 5);

    for (int i = 0; i < 5; ++i)
    {
        ar.set(i, i + 4);
    }

    for (int i = 0; i < 5; ++i)
    {
        DOCTEST_CHECK(ar[i].as<int32_t>() == i + 4);
    }

    auto vec = vecFromJSArray<int32_t>(ar);

    DOCTEST_CHECK(vec.size() == 5);
    for (int i = 0; i < 5; ++i)
    {
        DOCTEST_CHECK(vec[i] == i + 4);
    }

    local ar2 = ar;
    ar2.set(0, 11);
    DOCTEST_CHECK(ar2[0].as<int32_t>() == 11);
    DOCTEST_CHECK(ar[0].as<int32_t>() == 11);

#if !defined(JSBIND_CEF)
    // cef breaks this test case for some reason... it shouldn't
    local index(1);
    ar2.set(index, integer2);
    DOCTEST_CHECK(ar2[1].as<int32_t>() == 23);
    DOCTEST_CHECK(ar[1].as<int32_t>() == 23);

    // equals (not implementable in stupid cef v8)
    local strInt("23");
    DOCTEST_CHECK(strInt.equals(integer));
    DOCTEST_CHECK(!strInt.strictlyEquals(integer));

    local intInt(23);
    DOCTEST_CHECK(intInt.equals(integer));
    DOCTEST_CHECK(intInt.strictlyEquals(integer));
#endif

    DOCTEST_CHECK(test_handler->get_num_caught() == 0);
}

DOCTEST_TEST_CASE("persistent")
{
    persistent pi;
    persistent ps;
    persistent e;
    persistent ps2;

    {
        scope s;
        local integer(42);
        local str("bagavag");

        pi.reset(integer);
        ps.reset(str);
    }

    DOCTEST_CHECK(!pi.is_empty());
    DOCTEST_CHECK(!ps.is_empty());
    DOCTEST_CHECK(e.is_empty());
    DOCTEST_CHECK(ps2.is_empty());

    persistent tmp = e;

    DOCTEST_CHECK(tmp.is_empty());

    {
        scope s;
        local integer = pi.to_local();
        local str = ps.to_local();

        DOCTEST_CHECK(integer.as<int32_t>() == 42);
        DOCTEST_CHECK(str.as<std::string>() == "bagavag");

        pi.reset();
        DOCTEST_CHECK(pi.is_empty());

        ps2 = ps;
    }

    tmp = ps;

    ps.reset();

    DOCTEST_CHECK(ps.is_empty());
    DOCTEST_CHECK(e.is_empty());

    {
        scope s;
        local str = ps2.to_local();

        DOCTEST_CHECK(str.as<std::string>() == "bagavag");

        local str2 = tmp.to_local();

        DOCTEST_CHECK(str2.as<std::string>() == "bagavag");

        ps2.reset();
    }

    DOCTEST_CHECK(ps2.is_empty());
    DOCTEST_CHECK(!tmp.is_empty());

    DOCTEST_CHECK(test_handler->get_num_caught() == 0);
}

DOCTEST_TEST_CASE("global")
{
    scope s;

    run_script("g1 = 32.55; g2 = 'hippa-to'; g3 = { foo: 11, bar: 'asdf', baz: { x: 5, y: 6 } }; g4 = [8, 7, 6, 5];", "global");

    auto g = local::global();
    DOCTEST_CHECK(g.hasOwnProperty("g1"));
    DOCTEST_CHECK(g.hasOwnProperty("g2"));
    DOCTEST_CHECK(g.hasOwnProperty("g3"));
    DOCTEST_CHECK(g.hasOwnProperty("g4"));
    DOCTEST_CHECK(!g.hasOwnProperty("asdf"));

    auto g1 = local::global("g1");
    DOCTEST_CHECK(g1.as<uint32_t>() == 32);
    DOCTEST_CHECK(g1.typeOf().as<std::string>() == "number");
    DOCTEST_CHECK(Approx(g1.as<double>()) == 32.55);

    auto g2 = local::global("g2");
    DOCTEST_CHECK(g2.typeOf().as<std::string>() == "string");
    DOCTEST_CHECK(g2.as<std::string>() == "hippa-to");

    auto g3 = local::global("g3");
    DOCTEST_CHECK(g3.typeOf().as<std::string>() == "object");
    DOCTEST_CHECK(g3.hasOwnProperty("foo"));
    DOCTEST_CHECK(g3.hasOwnProperty("bar"));
    DOCTEST_CHECK(g3.hasOwnProperty("baz"));
    DOCTEST_CHECK(!g3.hasOwnProperty("x"));
    DOCTEST_CHECK(!g3.hasOwnProperty("y"));

    auto foo = g3["foo"];
    DOCTEST_CHECK(foo.typeOf().as<std::string>() == "number");
    DOCTEST_CHECK(foo.as<uint16_t>() == 11);

    auto bar = g3["bar"];
    DOCTEST_CHECK(bar.typeOf().as<std::string>() == "string");
    DOCTEST_CHECK(bar.as<std::string>() == "asdf");

    auto baz = g3["baz"];
    DOCTEST_CHECK(baz.typeOf().as<std::string>() == "object");
    DOCTEST_CHECK(baz.hasOwnProperty("x"));
    DOCTEST_CHECK(baz.hasOwnProperty("y"));
    DOCTEST_CHECK(!baz.hasOwnProperty("foo"));

    auto y = baz["y"];
    DOCTEST_CHECK(y.typeOf().as<std::string>() == "number");
    DOCTEST_CHECK(y.as<float>() == 6.f);

    int num_keys = 0;
    foreach(g3, [&num_keys](local key, local val) -> bool {
        auto skey = key.as<std::string>();

        if (skey == "foo")
        {
            DOCTEST_CHECK(val.as<int32_t>() == 11);
            ++num_keys;
        }
        else if (skey == "bar")
        {
            DOCTEST_CHECK(val.as<std::string>() == "asdf");
            ++num_keys;
        }
        else if (skey == "baz")
        {
            DOCTEST_CHECK(val.typeOf().as<std::string>() == "object");
            ++num_keys;
        }

        return true;
    });
    DOCTEST_CHECK(num_keys == 3);

    auto g4 = local::global("g4");
    DOCTEST_CHECK(g4.typeOf().as<std::string>() == "object");
    DOCTEST_CHECK(g4.hasOwnProperty("length"));
    DOCTEST_CHECK(!g4.hasOwnProperty("-1"));
    DOCTEST_CHECK(g4.hasOwnProperty("0"));
    DOCTEST_CHECK(g4.hasOwnProperty("3"));
    DOCTEST_CHECK(!g4.hasOwnProperty("4"));

    int i = 0;
    for (int n = 8; n >= 5; --n)
    {
        DOCTEST_CHECK(g4[i++].as<int8_t>() == n);
    }

    DOCTEST_CHECK(test_handler->get_num_caught() == 0);
}

DOCTEST_TEST_CASE("functions")
{
    scope s;

    run_script(
        "getFoo = function () { return 'foo'; };                    "
        "add5 = function (n) { return n + 5; };                     "
        "addFoo = function (str) { return str + 'foo'; };           "
        "vec = { x: 0, y: 0,                                        "
        "   init: function (x, y) { this.x = x; this.y = y; },      "
        "   manhattan: function () { return this.x + this.y; },     "
        "   dot: function (x, y) { return this.x * x + this.y * y; }"
        "};                                                         "
        , "functions");

    auto getFoo = local::global("getFoo");
    DOCTEST_CHECK(getFoo.typeOf().as<std::string>() == "function");

    local str = getFoo();
    DOCTEST_CHECK(str.as<std::string>() == "foo");

    auto add5 = local::global("add5");
    DOCTEST_CHECK(add5.typeOf().as<std::string>() == "function");

    local sum = add5(5);
    DOCTEST_CHECK(sum.as<int32_t>() == 10);

    sum = add5(11.5);
    DOCTEST_CHECK(Approx(sum.as<double>()) == 16.5);

    auto addFoo = local::global("addFoo");
    DOCTEST_CHECK(addFoo.typeOf().as<std::string>() == "function");

    local param("123");
    local food = addFoo(param);
    DOCTEST_CHECK(food.as<std::string>() == "123foo");

    auto vec = local::global("vec");
    vec.call<void>("init", 20, 22);
    DOCTEST_CHECK(vec.call<int32_t>("manhattan") == 42);

    vec.call<void>("init", 1.5, 2.1);
    DOCTEST_CHECK(Approx(vec.call<double>("dot", 3.14, 4.2)) == 13.53);

    DOCTEST_CHECK(test_handler->get_num_caught() == 0);
}

#if !defined(JSBIND_CEF)
DOCTEST_TEST_CASE("objects")
{
    scope s;

    run_script(
        "Vector = function(x, y) {                                    "
        "   this.x = x;                                               "
        "   this.y = y;                                               "
        "   this.length = function() {                                "
        "       return Math.sqrt(this.dot(this));                     "
        "   };                                                        "
        "   this.dot = function(other) {                              "
        "       return this.x * other.x + this.y * other.y;           "
        "   };                                                        "
        "   this.sum = function(other) {                              "
        "       return new Vector(this.x + other.x, this.y + other.y);"
        "   };                                                        "
        "}                                                            "
        , "objects");

    auto class_Vector = local::global("Vector");

    auto vec1 = class_Vector.new_(3, 4);
    DOCTEST_CHECK(vec1.typeOf().as<std::string>() == "object");
    DOCTEST_CHECK(vec1.call<int32_t>("length") == 5);

    auto vec2 = class_Vector.new_(-4, 3);
    DOCTEST_CHECK(vec2.typeOf().as<std::string>() == "object");
    DOCTEST_CHECK(vec2.call<int32_t>("length") == 5);

    DOCTEST_CHECK(vec1.call<int>("dot", vec2) == 0);

    auto sum = vec1.call<local>("sum", vec2);
    DOCTEST_CHECK(sum.typeOf().as<std::string>() == "object");
    DOCTEST_CHECK(sum["x"].as<float>() == -1.f);
    DOCTEST_CHECK(sum["y"].as<char>() == 7);

    DOCTEST_CHECK(test_handler->get_num_caught() == 0);
}
#endif

DOCTEST_TEST_CASE("proto")
{
    scope s;

    run_script(
        "Square = function() {};                "
        "Square.prototype.getType = function() {"
        "   return 'square';                    "
        "};                                     "
        "Square.prototype.typeId = 518;         "
        "square = new Square;                   "
        , "proto");

    auto square = local::global("square");
    DOCTEST_CHECK(square.typeOf().as<std::string>() == "object");

    // can't work with jsc
    //CHECK(!square.hasOwnProperty("typeId"));
    //CHECK(!square.hasOwnProperty("getType"));

    DOCTEST_CHECK(square.call<std::string>("getType") == "square");

    auto typeId = square["typeId"];

    DOCTEST_CHECK(typeId.as<int32_t>() == 518);

    DOCTEST_CHECK(test_handler->get_num_caught() == 0);
}

#if !defined(__EMSCRIPTEN__) && !defined(JSBIND_NODE)
class myconsole : public console
{
public:
    virtual void msg(console::msg_type type, const char* msg) override
    {
        switch (type)
        {
        case msg_log:
            m_log = msg;
            break;
        case msg_info:
            m_info = msg;
            break;
        case msg_warn:
            m_warn = msg;
            break;
        case msg_error:
            m_error = msg;
            break;
        case msg_debug:
            m_debug = msg;
            break;
        case msg_assert:
            m_assert = msg;
            break;
        default:
            DOCTEST_CHECK(false);
        }
    }

    std::string m_log, m_info, m_warn, m_error, m_debug, m_assert;
};

#if !defined(JSBIND_CEF)
DOCTEST_TEST_CASE("console")
{
    scope s;
    myconsole con;
    set_console(&con);
    run_script(
        "console.log('asd');"
        "console.info(1, 2);"
        "console.warn([1,   2]);"
        "console.error('foo' + 'bar');"
        "console.debug('foo', 'bar');"
        "console.assert(false, 'baz');"
        "console.assert(true, 'nonono');"
        , "console");

    DOCTEST_CHECK(con.m_log == "asd");
    DOCTEST_CHECK(con.m_info == "1 2");
    DOCTEST_CHECK(con.m_warn == "1,2");
    DOCTEST_CHECK(con.m_error == "foobar");
    DOCTEST_CHECK(con.m_debug == "foo bar");
    DOCTEST_CHECK(con.m_assert == "baz");

    DOCTEST_CHECK(test_handler->get_num_caught() == 0);
}
#endif

DOCTEST_TEST_CASE("except")
{
    scope s;
    run_script(
        "throwFoo = function(t) { if (t) throw 'foo'; };"
        "throwBar = function() { try { throw 'bar'; } catch (e) { return e; } return 'nothing'; };"
        , "except");

    auto foo = local::global("throwFoo");
    foo(false);
    DOCTEST_CHECK(test_handler->get_num_caught() == 0);

    foo(true);
    DOCTEST_CHECK(test_handler->get_num_caught() == 1);

    auto bar = local::global("throwBar");
    local ret = bar();
    DOCTEST_CHECK(test_handler->get_num_caught() == 0);
    DOCTEST_CHECK(ret.as<std::string>() == "bar");
}
#endif

DOCTEST_TEST_CASE("bind_static")
{
    scope s;
    person joe;
    joe.set_age(10);

    person jack;
    jack.set_age(20);

    run_script(
        "personUnde = Module.Person.initStatic();"
        "personClass = Module.Person.getClassName();"
        "numPersons = Module.Person.getNumPersons();"
        "meanPersonAge = Module.Person.getMeanAge();"
        "testInit = Module.TestClass.setStaticData('foo', 201, 23.14, 6.6);"
        );

    auto person_class = person::get_class_name();
    DOCTEST_CHECK(person_class == "Person-class");
    auto unde = local::global("personUnde");
    DOCTEST_CHECK(unde.isUndefined());

    auto personClass = local::global("personClass");
    DOCTEST_CHECK(personClass.as<std::string>() == person::get_class_name());

    auto numPersons = local::global("numPersons");
    DOCTEST_CHECK(numPersons.as<uint32_t>() == person::get_num_persons());

    auto meanPersonAge = local::global("meanPersonAge");
    DOCTEST_CHECK(meanPersonAge.as<float>() == person::get_mean_age());

    DOCTEST_CHECK(test::testclass::s_s == "foo");
    DOCTEST_CHECK(test::testclass::s_i == 201);
    DOCTEST_CHECK(Approx(test::testclass::s_f) == 23.14f);
    DOCTEST_CHECK(Approx(test::testclass::s_d) == 6.6);

    DOCTEST_CHECK(test_handler->get_num_caught() == 0);
}

DOCTEST_TEST_CASE("bind_pod")
{
    test::vec v = { 1.5f, 2.3f };
    test::store_vec(v);

    test::mec m = { 1, 0.2f };
    test::store_mec(m);

    test::sec s = { "pipi", 55 };
    test::store_sec(s);

    run_script(
        "vec = Module.getStoredVec();   "
        "vec.x += 2.2;                  "
        "vec.y += 3.3;                  "
        "Module.storeVec(vec);          "

        "mec = Module.getStoredMec();   "
        "mec.i -= 11;                   "
        "mec.f -= 0.3;                  "
        "Module.storeMec(mec);          "

        "sec = Module.getStoredSec();   "
        "sec.name += ' popo';           "
        "sec.age -= 40;                 "
        "Module.storeSec(sec);          "

        // "nec = Module.getStoredNec();   "
        // "Module.storeNec({});           "
        );

    v = test::get_stored_vec();
    DOCTEST_CHECK(Approx(v.x) == 3.7f);
    DOCTEST_CHECK(Approx(v.y) == 5.6f);

    m = test::get_stored_mec();
    DOCTEST_CHECK(m.i == -10);
    DOCTEST_CHECK(Approx(m.f) == -0.1f);

    s = test::get_stored_sec();
    DOCTEST_CHECK(s.name == "pipi popo");
    DOCTEST_CHECK(s.age == 15);
}

void jsbind_init_tests()
{
    test_handler = new test_exception_handler;
    jsbind::set_exception_handler(test_handler);
}

void jsbind_deinit_tests()
{
    jsbind::set_default_exception_handler();
    delete test_handler;
    test_handler = nullptr;
}
