// jsbind
// Copyright (c) 2019 Chobolabs Inc.
// http://www.chobolabs.com/
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// http://opensource.org/licenses/MIT
//
#pragma once

#include "jsbind/error.h"
#include "jsbind/common/index_sequence.h"
#include "jsbind/common/function_traits.h"
#include "convert.h"

#include <tuple>

namespace jsbind
{
namespace internal
{

    template <typename... Args>
    CefRefPtr<CefV8Value> cef_call(CefRefPtr<CefV8Value> self, CefRefPtr<CefV8Value> func, const Args&... args)
    {
        // +1 so when there are zero args we at least have something
        CefV8ValueList cef_args = { internal::to_cef(args)... };

        auto result = func->ExecuteFunction(self, cef_args);

        if (func->HasException())
        {
            report_exception(func->GetException());
        }

        return result;
    }

    ///////////////////////////////////////////////////////////////////

    template <typename Tuple, typename Func, size_t... Seq>
    typename std::enable_if<!is_void_return<Func>::value,
        CefRefPtr<CefV8Value>>::type tuple_call(Func func, const CefV8ValueList& args, index_sequence<Seq...>)
    {
        return to_cef(
            func(from_cef<typename std::tuple_element<Seq, Tuple>::type>(args[Seq]) ...)
        );
    }

    template <typename Tuple, typename Func, size_t... Seq>
    typename std::enable_if<is_void_return<Func>::value,
        CefRefPtr<CefV8Value>>::type tuple_call(Func func, const CefV8ValueList& args, index_sequence<Seq...>)
    {
        func(from_cef<typename std::tuple_element<Seq, Tuple>::type>(args[Seq]) ...);
        return CefV8Value::CreateUndefined();
    }


    template <typename ReturnType, typename... Args>
    class cef_handler : public CefV8Handler
    {
        IMPLEMENT_REFCOUNTING(cef_handler);

    public:
        cef_handler(ReturnType(*cxx_func)(Args...))
            : m_cxx_func(cxx_func)
        {}

        virtual bool Execute(const CefString& name, CefRefPtr<CefV8Value> object, const CefV8ValueList& arguments, CefRefPtr<CefV8Value>& retval, CefString& exception) override
        {
            JSBIND_JS_CHECK((unsigned long)arguments.size() >= sizeof...(Args), "Not enough arguments for function.");
            retval = tuple_call<std::tuple<Args...>>(m_cxx_func, arguments, make_index_sequence<sizeof...(Args)>());
            return true;
        }

        ReturnType(*m_cxx_func)(Args...);
    };
}
}
