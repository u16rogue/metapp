#pragma once

#define _mpp_glue_proxy(x, y) x ## y
#define mpp_glue(x, y) _mpp_glue_proxy(x, y)

namespace mpp {

namespace details {

template <typename T>
struct __mpp_scope_in {
  __mpp_scope_in(T && fn) {
    fn();
  }
};

template <typename T>
struct __mpp_scope_out {
  __mpp_scope_out(T && fn_) : fn(fn_) {}
  ~__mpp_scope_out() { fn(); }
  T fn;
};

} // namespace details


using  i8  = char;               static_assert(sizeof(i8)  * 8 == 8  , "Data type i8 did not match its expected type size.");
using  u8  = unsigned char;      static_assert(sizeof(u8)  * 8 == 8  , "Data type u8 did not match its expected type size.");
using  i16 = short;              static_assert(sizeof(i16) * 8 == 16 , "Data type i16 did not match its expected type size.");
using  u16 = unsigned short;     static_assert(sizeof(u16) * 8 == 16 , "Data type u16 did not match its expected type size.");
using  i32 = int;                static_assert(sizeof(i32) * 8 == 32 , "Data type i32 did not match its expected type size.");
using  u32 = unsigned int;       static_assert(sizeof(u32) * 8 == 32 , "Data type u32 did not match its expected type size.");
using  i64 = long long;          static_assert(sizeof(i64) * 8 == 64 , "Data type i64 did not match its expected type size.");
using  u64 = unsigned long long; static_assert(sizeof(u64) * 8 == 64 , "Data type u64 did not match its expected type size.");

//=========================================================================================
//=== Compile time string

template <int sz>
struct CompStr {
  consteval CompStr(const char (&str_)[sz]) {
    for (int i = 0; i < sz; ++i)
      _data[i] = str_[i];
  }

  constexpr auto size() -> int {
    return sz;
  }

  constexpr auto length() -> int {
    return sz - 1;
  }

  constexpr auto data() -> const char * {
    return _data;
  }

private:
  char _data[sz] {};
};

} // namespace mpp
