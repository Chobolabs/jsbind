#pragma once

#include <emscripten/val.h>
#include <functional>

namespace jsbind
{

struct scope {};

typedef emscripten::val local;

template <typename Func>
// here we should have function<bool(local, local)> instead of typename Func
// but an emscripten bug causes a crash in this case
void foreach(local obj, Func iteration)
{
    auto keys = local::global("Object")["keys"](obj);
    auto len = keys["length"].as<int>();

    for (int i=0; i<len; ++i)
    {
        auto key = keys[i];
        auto value = obj[key];
        if (!iteration(key, value))
        {
            break;
        }
    }
}

class persistent
{
public:
    persistent()
        : m_is_empty(true)
        , m_local(local::undefined())
    {}

    explicit persistent(const local& local)
        : m_is_empty(false)
        , m_local(local)
    {}

    void reset()
    {
        m_is_empty = true;
        m_local = local::undefined();
    }

    void reset(const local& l)
    {
        m_is_empty = false;
        m_local = l;
    }

    local to_local() const
    {
        return m_local;
    }

    bool is_empty() const
    {
        return m_is_empty;
    }

private:
    bool m_is_empty; // could be hacked around with local from internal::EM_VAL nullptr
    local m_local;
};

using ::emscripten::vecFromJSArray;

}
