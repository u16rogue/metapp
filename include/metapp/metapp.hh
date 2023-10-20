#pragma once

#include "common.hh"

//=========================================================================================
//=== Comparison dispatch
// TODO
//=========================================================================================
//=== Enum class bitwise operators
// TODO
//=========================================================================================
//=== Fixed datatypes
// TODO: static_assert to ensure data type is correct
namespace mpp
{

template <typename T>
concept ConstraintHasArrowOperator = requires (T a) {
  a.operator->();
};

using  i8  = char;
struct I8 { i8 value; };
static_assert(
  sizeof(i8) * 8 == 8 && sizeof(I8) * 8 == 8,
  "Data type i8 and U8 did not match its expected type size.");

using  u8  = unsigned char;
struct U8 { u8 value; };
static_assert(
  sizeof(u8) * 8 == 8 && sizeof(U8) * 8 == 8,
  "Data type u8 and U8 did not match its expected type size.");

using  i16 = short;
struct I16 { i16 value; };
static_assert(
  sizeof(i16) * 8 == 16 && sizeof(I16) * 8 == 16,
  "Data type i16 and I16 did not match its expected type size.");

using  u16 = unsigned short;
struct U16 { u16 value; };
static_assert(
  sizeof(u16) * 8 == 16 && sizeof(U16) * 8 == 16,
  "Data type u16 and U16 did not match its expected type size.");

using  i32 = int;
struct I32 { i32 value; };
static_assert(
  sizeof(i32) * 8 == 32 && sizeof(I32) * 8 == 32,
  "Data type i32 and I32 did not match its expected type size.");

using  u32 = unsigned int;
struct U32 { u32 value; };
static_assert(
  sizeof(u32) * 8 == 32 && sizeof(U32) * 8 == 32,
  "Data type u32 and U32 did not match its expected type size.");

using  i64 = long long;
struct I64 { i64 value; };
static_assert(
  sizeof(i64) * 8 == 64 && sizeof(I64) * 8 == 64,
  "Data type i64 and I64 did not match its expected type size.");

using  u64 = unsigned long long;
struct U64 { u64 value; };
static_assert(
  sizeof(u64) * 8 == 64 && sizeof(U64) * 8 == 64,
  "Data type u64 and U64 did not match its expected type size.");

//=========================================================================================
//=== Defer statement
namespace details
{
  template <typename T>
  struct __mpp_defer
  {
    __mpp_defer(T && fn_) : fn(fn_) {}
    ~__mpp_defer() { fn(); }
    T fn;
  };
} // details

#define mpp_defer \
  const mpp::details::__mpp_defer mpp_glue(__mpp_defer_, __LINE__)= [&]()

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

  constexpr auto fnv1a64_partial(const char * str, const int exp_len) -> mpp::u64 {
    const char * c= str;
    str += exp_len;
    mpp::u64 h= 0xcbf29ce484222325;
    while (*c && (c != str)) {
      h= (h ^ *c) * 0x00000100000001B3;
      ++c;
    }
    return h;
  }

  constexpr auto fnv1a64(const char * str) -> mpp::u64 {
    mpp::u64 h= 0xcbf29ce484222325;
    while (*str) {
      h= (h ^ *str) * 0x00000100000001B3;
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

//=========================================================================================
//=== Results

namespace details::result {
  enum class Status {
    SUCCESS = 0,
    FAILURE = 1, // Unspecified failure
  };
} // details::result

/*
 *  Optional type with alternative value for lack of `T` presented by `S`
 */
template <typename T, typename S = details::result::Status, bool custom_arrow_operator_chaining = true>
struct Result {
  using StatusType = S;
  static_assert(requires { S::SUCCESS; } && static_cast<int>(S::SUCCESS) == 0, "Invalid Status enumerator `S`. Must contain 'SUCCESS' and its value must be 0.");
  static_assert(requires { S::FAILURE; } && static_cast<int>(S::FAILURE) == 1, "Invalid Status enumerator `S`. Must contain 'FAILURE' and its value must be 1.");

  Result()
    : is_success(false), as_failure(S::FAILURE)
  {}

  // Direct `T` constructor
  template <typename... VArgs>
  Result(VArgs &&... args)
    : is_success(true)
  {
    new (&success_as_value_typed()) T(static_cast<VArgs&&>(args)...);
  }

  // Move T constructor
  Result(T && value)
    : is_success(true)
  {
    new (&success_as_value_typed()) T(static_cast<T &&>(value));
  }

  // Copy T constructor
  Result(const T & value)
    : is_success(true)
  {
    new (&success_as_value_typed()) T(value);
  }

  // Move constructor
  Result(Result && other) {
    success_destructorcall_checked();
    if (other.is_success) {
      new (&success_as_value_typed()) T(static_cast<T &&>(other.success_as_value_typed()));
      is_success = true;

      other.is_success = false;
      other.as_failure = S::FAILURE;
    } else {
      is_success = false;
      as_failure = other.as_failure;
    }
  }

  // Copy constructor
  Result(const Result & other) {
    success_destructorcall_checked();
    if (other.is_success) {
      new (&success_as_value_typed()) T(other);
      is_success = true;
    } else {
      as_failure = other.as_failure;
      is_success = false;
    }
  }

  // Error constructor
  Result(S status)
    : is_success(false)
  {
    if (status == S::SUCCESS) {
      status = S::FAILURE;
    }
    as_failure = status;
  }

  ~Result() {
    success_destructorcall_checked();
  }

  explicit operator bool() const noexcept {
    return successful();
  }

  auto operator->() -> T & requires (ConstraintHasArrowOperator<T> && custom_arrow_operator_chaining) {
    return success_as_value_typed();
  }

  auto operator->() -> T * {
    return &success_as_value_typed();
  }

  auto operator*() -> T & {
    return success_as_value_typed();
  }

  auto successful() const noexcept -> bool {
    return is_success;
  }

  auto why() const noexcept -> S {
    if (!is_success) {
      return as_failure;
    } else {
      return S::SUCCESS;
    }
  }

private:
  bool is_success;
  union {
    S  as_failure;
    u8 as_success[sizeof(T)];
  };
  static_assert(sizeof(as_success) == sizeof(T), "Pseudo container for type `T` isn't properly aligned.");

private:
  auto success_as_value_typed() -> T & {
    return *reinterpret_cast<T*>(&as_success);
  }
  auto success_destructorcall_checked() -> void {
    if (is_success) {
      success_as_value_typed().~T();
    }
  }
}; // struct Result

} // mpp

