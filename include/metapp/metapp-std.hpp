#pragma once

#include "common.hpp"

// -----------------------------------------------------------------------------------------
// --- Defer statement

#if defined(METAPP_LOCK_USE_LINE_NUMBER) && METAPP_LOCK_USE_LINE_NUMBER == 1
#define mpp_lock(mutx) \
  const mpp::details::__mpp_imm mpp_glue(__mpp_lock_, __LINE__) = [&, lk = std::lock_guard<std::mutex>(mutx)]()
#else
#define mpp_lock(mutx) \
  const mpp::details::__mpp_imm mpp_glue(__mpp_lock_, __COUNTER__) = [&, lk = std::lock_guard<std::mutex>(mutx)]()
#endif
// -----------------------------------------------------------------------------------------
