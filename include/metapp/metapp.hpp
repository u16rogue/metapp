#pragma once

#include "common.hpp"

// -----------------------------------------------------------------------------------------
// --- Comparison dispatch
// TODO
// -----------------------------------------------------------------------------------------
// --- Enum class bitwise operators
// TODO
// -----------------------------------------------------------------------------------------
// --- Fixed datatypes
// TODO: static_assert to ensure data type is correct
namespace mpp
{
using i8  = char;
using i16 = short;
using i32 = int;
using i64 = long long;

using u8  = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;
using u64 = unsigned long long;
}
// -----------------------------------------------------------------------------------------
// --- Defer statement
namespace mpp::details
{

template <typename T>
struct __mpp_defer
{
  __mpp_defer(T && fn_) : fn(fn_) {}
  ~__mpp_defer() { fn(); }
  T fn;
};
  
}

#if defined(METAPP_DEFER_USE_LINE_NUMBER) && METAPP_DEFER_USE_LINE_NUMBER == 1
  #define mpp_defer \
    const mpp::details::__mpp_defer mpp_glue(__mpp_defer_, __LINE__) = [&]()
#else
  #define mpp_defer \
    const mpp::details::__mpp_defer mpp_glue(__mpp_defer_, __COUNTER__) = [&]()
#endif
// -----------------------------------------------------------------------------------------
// --- Array size
namespace mpp
{

template <typename T, int sz>
constexpr auto array_size(const T(&)[sz]) -> mpp::u64
{
  return sz;
}

}
// -----------------------------------------------------------------------------------------
// --- Normalize Lambda
namespace mpp
{
namespace details
{
template <typename T, typename R, typename... P>
struct abi_provider
{
  inline static T * fn;
  abi_provider(R(T::*)(P...)) {};
  static auto abi(P... p) -> R
  {
    return fn->operator()(p...);
  }
};

template <typename T>
struct lambda_abi
{
  inline static auto provider = abi_provider(&T::operator());
  lambda_abi(T && lambda) {
    static T fn = lambda;
    provider.fn = &fn;
  }
};
}

template <typename T>
auto normalize_lambda_now(T && lambda) -> auto
{
return details::lambda_abi<T>(static_cast<T &&>(lambda)).provider.abi;
}

/*
template <typename T>
auto normalize_lambda_from(T & lambda) -> auto
{}
*/
}

