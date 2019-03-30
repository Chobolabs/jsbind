// jsbind
// Copyright (c) 2019 Chobolabs Inc.
// http://www.chobolabs.com/
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// http://opensource.org/licenses/MIT
//
#pragma once

#include "convert.hpp"
#include "jsbind/error.hpp"
#include "jsbind/common/index_sequence.hpp"
#include "jsbind/common/function_traits.hpp"

#include <tuple>

namespace jsbind
{
namespace internal
{

    template <typename... Args>
    JSValueRef jsc_call(JSObjectRef self, JSObjectRef func, const Args&... args)
    {
        const int num_args = sizeof...(Args);
        // +1 so when there are zero args we at least have something
        JSValueRef jsc_args[num_args + 1] = { internal::to_jsc(args)... };

        JSValueRef exception = nullptr;
        auto ret = JSObjectCallAsFunction(jsc_context, func, self, num_args, jsc_args, &exception);

        if (exception) report_exception(exception);

        return ret;
    }

    ///////////////////////////////////////////////////////////////////////////

    template <typename Tuple, typename Func, size_t... Seq>
    typename std::enable_if<!is_void_return<Func>::value,
        JSValueRef>::type tuple_call(Func func, const JSValueRef args[], index_sequence<Seq...>)
    {
        return to_jsc(
            func(from_jsc<typename std::tuple_element<Seq, Tuple>::type>(args[Seq]) ...)
            );
    }

    template <typename Tuple, typename Func, size_t... Seq>
    typename std::enable_if<is_void_return<Func>::value,
        JSValueRef>::type tuple_call(Func func, const JSValueRef args[], index_sequence<Seq...>)
    {
        func(from_jsc<typename std::tuple_element<Seq, Tuple>::type>(args[Seq]) ...);
        return JSValueMakeUndefined(jsc_context);
    }

    struct function_private_data
    {
        virtual ~function_private_data() {}
        virtual JSValueRef call(JSObjectRef self, size_t numArgs, const JSValueRef args[]) = 0;
    };

    template <typename ReturnType, typename... Args>
    struct call_class_function_from_jsc : public function_private_data
    {
        call_class_function_from_jsc(ReturnType(*cxx_func)(Args...))
            : m_cxx_func(cxx_func)
        {}

        virtual JSValueRef call(JSObjectRef self, size_t numArgs, const JSValueRef args[]) override
        {
            JSBIND_JS_CHECK(numArgs >= sizeof...(Args), "Not enough arguments for function.");

            return tuple_call<std::tuple<Args...>>(m_cxx_func, args, make_index_sequence<sizeof...(Args)>());
        }

        ReturnType(*m_cxx_func)(Args...);
    };

    inline JSValueRef call_from_jsc(
        JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject,
        size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
    {
        auto fdata = reinterpret_cast<function_private_data*>(JSObjectGetPrivate(function));
        assert(fdata && "No private data for function");
        return fdata->call(thisObject, argumentCount, arguments);
    }
}
}
