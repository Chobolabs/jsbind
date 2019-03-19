// jsbind
// Copyright (c) 2019 Chobolabs Inc.
// http://www.chobolabs.com/
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// http://opensource.org/licenses/MIT
//
#pragma once

namespace jsbind
{
namespace internal
{
    // code lifted from v8pp

    template<typename Func>
    struct function_traits;

    template<typename R, typename ...Args>
    struct function_traits<R(Args...)>
    {
        using return_type = R;
        using arguments = std::tuple<Args...>;
    };

    // function pointer
    template<typename R, typename ...Args>
    struct function_traits<R(*)(Args...)> : function_traits<R(Args...)>{};

    // member function pointer
    template<typename C, typename R, typename ...Args>
    struct function_traits<R(C::*)(Args...)> : function_traits<R(C&, Args...)>
    {
    };

    // const member function pointer
    template<typename C, typename R, typename ...Args>
    struct function_traits<R(C::*)(Args...) const> : function_traits<R(const C&, Args...)>
    {
    };

    // volatile member function pointer
    template<typename C, typename R, typename ...Args>
    struct function_traits<R(C::*)(Args...) volatile> : function_traits<R(volatile C&, Args...)>
    {
    };

    // const volatile member function pointer
    template<typename C, typename R, typename ...Args>
    struct function_traits<R(C::*)(Args...) const volatile> : function_traits<R(const volatile C&, Args...)>
    {
    };

    // member object pointer
    template<typename C, typename R>
    struct function_traits<R(C::*)> : function_traits<R(C&)>
    {
    };

    ///////////////////////////////////////////////////////////////////////////

    template<typename F>
    using is_void_return = std::is_same<void, typename function_traits<F>::return_type>;
}
}