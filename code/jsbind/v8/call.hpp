// jsbind
// Copyright (c) 2019 Chobolabs Inc.
// http://www.chobolabs.com/
//
// Distributed under the MIT Software License
// See accompanying file LICENSE.txt or copy at
// http://opensource.org/licenses/MIT
//
#pragma once

#include "jsbind/error.hpp"
#include "jsbind/common/index_sequence.hpp"
#include "jsbind/common/function_traits.hpp"
#include "convert.hpp"

#include <tuple>

namespace jsbind
{
namespace internal
{

    template <typename... Args>
    v8::Local<v8::Value> v8_call(v8::Local<v8::Value> self, v8::Function* func, const Args&... args)
    {
        v8::EscapableHandleScope scope(internal::isolate);

        const int num_args = sizeof...(Args);
        // +1 so when there are zero args we at least have something
        v8::Local<v8::Value> v8_args[num_args + 1] = { internal::to_v8(args)... };

        v8::TryCatch tc(internal::isolate);

        auto& v8ctx = *reinterpret_cast<v8::Local<v8::Context>*>(&internal::ctx.v8ctx);
        auto result = func->Call(v8ctx, self, num_args, v8_args);

        if (tc.HasCaught()) report_exception(tc);

        if (result.IsEmpty()) return v8::Undefined(internal::isolate);

        return scope.Escape(result.ToLocalChecked());
    }

    ///////////////////////////////////////////////////////////////////

    template <typename Tuple, typename Func, size_t... Seq>
    typename std::enable_if<!is_void_return<Func>::value,
        void>::type tuple_call(Func func, const v8::FunctionCallbackInfo<v8::Value>& args, index_sequence<Seq...>)
    {
        args.GetReturnValue().Set(to_v8(
            func(from_v8<typename std::tuple_element<Seq, Tuple>::type>(args[Seq]) ...)
            ));
    }

    template <typename Tuple, typename Func, size_t... Seq>
    typename std::enable_if<is_void_return<Func>::value,
        void>::type tuple_call(Func func, const v8::FunctionCallbackInfo<v8::Value>& args, index_sequence<Seq...>)
    {
        func(from_v8<typename std::tuple_element<Seq, Tuple>::type>(args[Seq]) ...);
    }

    template <typename ReturnType, typename... Args>
    void call_class_function_from_v8(const v8::FunctionCallbackInfo<v8::Value>& args)
    {
        void* data = args.Data().As<v8::External>()->Value();
        auto func = reinterpret_cast<ReturnType (*)(Args...)>(data);
        JSBIND_JS_CHECK((unsigned long)args.Length() >= sizeof...(Args), "Not enough arguments for function.");

        tuple_call<std::tuple<Args...>>(func, args, make_index_sequence<sizeof...(Args)>());
    }
}
}
