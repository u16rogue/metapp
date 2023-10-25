#pragma once

#include <metapp/common.hh>

namespace mpp
{

struct I8 { i8 value; };
static_assert(sizeof(I8) * 8 == 8, "Data type I8 did not match its expected type size.");

struct U8 { u8 value; };
static_assert(sizeof(U8) * 8 == 8, "Data type U8 did not match its expected type size.");

struct I16 { i16 value; };
static_assert(sizeof(I16) * 8 == 16, "Data type I16 did not match its expected type size.");

struct U16 { u16 value; };
static_assert(sizeof(U16) * 8 == 16, "Data type U16 did not match its expected type size.");

struct I32 { i32 value; };
static_assert(sizeof(I32) * 8 == 32, "Data type I32 did not match its expected type size.");

struct U32 { u32 value; };
static_assert(sizeof(U32) * 8 == 32, "Data type U32 did not match its expected type size.");

struct I64 { i64 value; };
static_assert(sizeof(I64) * 8 == 64, "Data type I64 did not match its expected type size.");

struct U64 { u64 value; };
static_assert(sizeof(U64) * 8 == 64, "Data type U64 did not match its expected type size.");

//=========================================================================================
//=== Defer statement

#define mpp_defer \
  const mpp::details::__mpp_scope_out mpp_glue(__mpp_defer_, __LINE__)= [&]()

//=========================================================================================
//=== Array length
//
// [15/10/2023 - rogue] NOTICE: Changed from size to length for better terminology.
// size= size in bytes
// count= number of entries (active)
// length= number of elements (all)
template <typename T, int sz>
constexpr auto array_length(const T(&)[sz]) -> mpp::u64
{
  return sz;
}

//=========================================================================================
//=== Normalize Lambda
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
    inline static auto provider= abi_provider(&T::operator());
    lambda_abi(T && lambda) {
      static T fn= lambda;
      provider.fn= &fn;
    }
  };
} // details

template <typename T>
auto normalize_lambda_now(T && lambda) -> auto
{
  return details::lambda_abi<T>(static_cast<T &&>(lambda)).provider.abi;
}

//=========================================================================================
//=== Comptime String hash (FNV A1)

// TODO: Optimization idea is to receive expected length then calculate the rhs string's length while hashing,
//       lemgth is compared by the delta between the starting cstr pointer and the current cstr. This elimantes
//       the precall to details::strlen for the operator=='s
namespace details {

  constexpr auto fnv1a64_partial(const char * str, const mpp::u64 exp_len) -> mpp::u64 {
    const char * c = str;
    str += exp_len;
    mpp::u64 h = 0xcbf29ce484222325;
    while (*c && (c != str)) {
      h= (h ^ static_cast<mpp::u64>(*c)) * 0x00000100000001B3;
      ++c;
    }
    return h;
  }

  constexpr auto fnv1a64(const char * str) -> mpp::u64 {
    mpp::u64 h = 0xcbf29ce484222325;
    while (*str) {
      h= (h ^ static_cast<mpp::u64>(*str)) * 0x00000100000001B3;
      ++str;
    }
    return h;
  }

  constexpr auto strlen(const char * str) -> mpp::u64 {
    mpp::u64 l= 0;
    while (str[++l]);
    return l;
  }

} // details

struct CmpHStr {
  consteval CmpHStr(const char * str) {
    v=  details::fnv1a64(str);
  }

  constexpr auto operator==(const CmpHStr rhs) const -> bool {
    return v== rhs.v;
  }

  constexpr auto operator==(const char * rhs) const -> bool {
    return v== details::fnv1a64(rhs);
  }

  mpp::u64 v;
};

struct CmpHStrPartial {
  consteval CmpHStrPartial(const char * str) {
    l= details::strlen(str);
    v= details::fnv1a64(str);
  }

  constexpr auto operator==(const CmpHStrPartial rhs) const -> bool {
    return v== rhs.v;
  }

  constexpr auto operator==(const char * rhs) const -> bool {
    return v== details::fnv1a64_partial(rhs, l);
  }

  mpp::u64 v;
  mpp::u64 l;
};

} // mpp

