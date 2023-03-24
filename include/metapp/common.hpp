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

}
