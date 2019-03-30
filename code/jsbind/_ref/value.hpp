// jsbind
// Copyright (c) 2019 Chobolabs Inc.
// http://www.chobolabs.com/
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// http://opensource.org/licenses/MIT
//
#pragma once

#include <vector>
#include <functional>

namespace jsbind
{

class scope
{
public:
    scope();
};

class local
{
public:
    local() = delete;

    template <typename T>
    explicit local(const T& val);

    static local null();

    static local undefined();

    static local object();

    static local array();

    static local global(const char* name = nullptr);

    bool isNull() const;

    bool isUndefined() const;

    bool isTrue() const;

    bool isFalse() const;

    bool hasOwnProperty(const char* key) const;

    template <typename K>
    local operator[](const K& key) const;

    template <typename K, typename V>
    void set(const K& key, const V& val);

    bool equals(const local& other) const;

    bool strictlyEquals(const local& other) const;

    template <typename... Args>
    local new_(const Args&... args);

    template <typename... Args>
    local operator()(const Args&... args);

    template <typename ReturnType, typename... Args>
    ReturnType call(const char* name, const Args&... args);

    template <typename T>
    T as() const;

    local typeOf() const;
};

template<typename T>
std::vector<T> vecFromJSArray(local v);

inline void foreach(local obj, std::function<bool(local key, local value)> iteration);

class persistent
{
public:
    persistent();

    explicit persistent(const local& local);

    void reset();

    void reset(const local& l);

    local to_local() const;

    bool is_empty() const;
};

}
