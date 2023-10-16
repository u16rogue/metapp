#pragma once

#define _mpp_glue_proxy(x, y) x ## y
#define mpp_glue(x, y) _mpp_glue_proxy(x, y)

namespace mpp::details {

template <typename T>
struct __mpp_imm {
  __mpp_imm(T && fn) {
    fn();
  }
};

} // namespace mpp::details

namespace mpp {

// Compile time string
template <int sz>
struct compstr {
  consteval compstr(const char (&str_)[sz]) {
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
