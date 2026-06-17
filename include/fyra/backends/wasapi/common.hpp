#pragma once

#include <memory>
#include <stdexcept>

#include <Audioclient.h>
#include <Windows.h>
#include <mmdeviceapi.h>

#include "fyra/utility/function_traits.hpp"

namespace fyra::backends::wasapi
{

template <typename T>
struct wasapi_deleter
{
    void operator()(T* const ptr) const
    {
        if (ptr)
            ptr->Release();
    }
};

template <>
struct wasapi_deleter<WAVEFORMATEX>
{
    void operator()(WAVEFORMATEX* const ptr) const
    {
        CoTaskMemFree(ptr);
    }
};

template <typename T>
using wasapi_ptr = std::unique_ptr<T, wasapi_deleter<T>>;

template <typename Fn_forward, typename... Args>
decltype(auto) com_call(Fn_forward&& func, Args&&... args)
{
    if (FAILED(std::invoke(std::forward<Fn_forward>(func), std::forward<Args>(args)...)))
        throw std::runtime_error("COM call failed!");
}

template <typename Fn_forward, typename... Args>
decltype(auto) com_call_no_alloc(Fn_forward&& func, Args&&... args)
{
    using Fn = std::decay_t<Fn_forward>;
    using Traits = fyra::utility::function_traits<Fn>;
    static constexpr size_t args_count = std::tuple_size_v<typename Traits::args_tuple>;
    static_assert(args_count > 0, "Invalid number of arguments");

    static constexpr size_t last_index = args_count - 1;

    using OutType = std::remove_pointer_t<typename Traits::template arg_at<last_index>>;
    OutType temp;
    if (FAILED(std::invoke(std::forward<Fn_forward>(func), std::forward<Args>(args)..., &temp)))
        throw std::runtime_error("COM call failed!");

    return temp;
}

template <typename Fn_forward, typename... Args>
decltype(auto) com_call_wptr(Fn_forward&& func, Args&&... args)
{
    using Fn = std::decay_t<Fn_forward>;
    using Traits = fyra::utility::function_traits<Fn>;
    static constexpr size_t args_count = std::tuple_size_v<typename Traits::args_tuple>;
    static_assert(args_count > 0, "Invalid number of arguments");

    static constexpr size_t last_index = args_count - 1;

    using OutType = std::remove_pointer_t<typename Traits::template arg_at<last_index>>;
    using ReturnType = wasapi_ptr<std::remove_pointer_t<OutType>>;

    OutType temp{nullptr};
    if (FAILED(std::invoke(std::forward<Fn_forward>(func), std::forward<Args>(args)..., &temp)))
        throw std::runtime_error("COM call failed!");

    return ReturnType{temp};
}

template <typename Casted, typename Fn_forward, typename... Args>
decltype(auto) com_call_wptr(Fn_forward&& func, Args&&... args)
{
    using Fn = std::decay_t<Fn_forward>;
    using Traits = fyra::utility::function_traits<Fn>;
    static constexpr size_t args_count = std::tuple_size_v<typename Traits::args_tuple>;
    static_assert(args_count > 0, "Invalid number of arguments");

    static constexpr size_t last_index = args_count - 1;

    using OutType = std::remove_pointer_t<typename Traits::template arg_at<last_index>>;
    using ReturnType = wasapi_ptr<Casted>;

    OutType temp{nullptr};
    if (FAILED(std::invoke(std::forward<Fn_forward>(func), std::forward<Args>(args)..., &temp)))
        throw std::runtime_error("COM call failed!");

    return ReturnType{static_cast<Casted*>(temp)};
}

} // namespace fyra::backends::wasapi