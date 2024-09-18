#pragma once

#include <metapp/common.hh>
#include <metapp/concepts.hh>
//#include <new>
//#include <type_traits>

#define mpp_lock(mutx) \
  const mpp::details::__mpp_scope_in mpp_glue(__mpp_lock_, __LINE__) = [&, lk = std::lock_guard<std::mutex>(mutx)]()

namespace mpp {

namespace details::result {
  enum class Reason : mpp::u8 {
    Failure,
    ResultInvalidated,
  };
} // mpp::details::result

/*
 *  Optional type with alternative value for lack of `T` presented by `R`
 */
template <typename T, typename R = details::result::Reason, bool custom_arrow_operator_chaining = true>
struct Result {
  using ReasonType = R;

  static_assert(requires { R::ResultInvalidated; }, "Result::ReasonType of R must have a `ResultInvalidated` enumerator.");

  // Direct `T` constructor
  template <typename... VArgs>
  constexpr Result(VArgs &&... args)
    : is_ok(true)
  {
    _unsafe_vT_constructor(static_cast<VArgs>(args)...);
  }

  // Move T constructor
  constexpr Result(T && value)
    : is_ok(true)
  {
    _unsafe_vT_move_from(static_cast<T&&>(value));
  }

  // Copy T constructor
  constexpr Result(T & value)
    : is_ok(true)
  {
    _unsafe_vT_copy_from(value);
  }

  // Move semantics
  constexpr Result(Result && other)
    : is_ok(false)
  {
    _checked_move_semantic(static_cast<Result&&>(other));
  }

  constexpr auto operator=(Result && other) -> Result& {
    return _checked_move_semantic(static_cast<Result&&>(other));
  }

  // Copy constructor
  constexpr Result(Result & other)
    : is_ok(false)
  {
    _checked_copy_semantic(other);
  }

  constexpr auto operator=(Result & other) -> Result& {
    return _checked_copy_semantic(other);
  }

  // Error constructor
  constexpr Result(R reason)
    : is_ok(false),
      vR(reason)
  {}

  constexpr ~Result() {
    _checked_invalidate();
  }

  constexpr auto ok() const noexcept -> bool {
    return is_ok;
  }

  constexpr explicit operator bool() const noexcept {
    return ok();
  }

  constexpr auto has_value() const noexcept -> bool {
    return ok();
  }

  constexpr auto has_reason() const noexcept -> bool {
    return !ok() && has_R;
  }

  constexpr auto operator->() -> T& requires (ConstraintHasArrowOperator<T> && custom_arrow_operator_chaining) {
    return _unsafe_vT_ref();
  }

  constexpr auto operator*() -> T& {
    return _unsafe_vT_ref();
  }

  constexpr auto operator->() -> T* {
    return &_unsafe_vT_ref();
  }

  constexpr auto access_result(const auto && callback) /*const*/ noexcept -> Result& {
    if (ok()) {
      callback(_unsafe_vT_ref());
    }
    return *this;
  }

  constexpr auto try_result(const auto && callback, auto & fallback) /*const*/ noexcept -> auto& {
    if (ok()) {
      return callback(_unsafe_vT_ref());
    } else {
      return fallback;
    }
  }

  constexpr auto access_reason(const auto && callback) /*const*/ noexcept -> Result& {
    if (has_reason()) {
      callback(vR);
    }
    return *this;
  }

  constexpr auto try_reason(const auto && callback, auto & fallback) /*const*/ noexcept -> auto& {
    if (has_reason()) {
      return callback(vR);
    } else {
      return fallback;
    }
  }

  constexpr auto visit_all(const auto &&... callbacks) const noexcept -> auto requires (false) {
  }

private:
  struct {
    bool is_ok : 1;
    bool has_R : 1; // Marked when Result is invalidated and ResultInvalidated is unavailable.
    char _pad  : 6;
  };
  union {
    R  vR;
    u8 _raw_vT[sizeof(T)];
  };
  static_assert(sizeof(_raw_vT) == sizeof(T), "Pseudo container for type `T` isn't properly aligned.");

private:
  auto _unsafe_vT_ref() -> T& {
    return *reinterpret_cast<T*>(&_raw_vT);
  }

  auto _unsafe_vT_ref() const -> const T& {
    return *reinterpret_cast<const T*>(&_raw_vT);
  }

  auto _unsafe_reason() const noexcept -> R {
    return vR;
  }

  template <typename... VArgs>
  auto _unsafe_vT_constructor(VArgs &&... args) {
    new (&_unsafe_vT_ref()) T(static_cast<VArgs&&>(args)...);
  }

  auto _unsafe_vT_destructor() -> void {
    _unsafe_vT_ref().~T();
  }

  auto _unsafe_vT_move_from(T && other) -> void {
    new (&_unsafe_vT_ref()) T(static_cast<T&&>(other));
  }

  auto _unsafe_vT_copy_from(T & other) -> void {
    new (&_unsafe_vT_ref()) T(other);
  }

  auto _checked_vT_destructor() -> void {
    if (is_ok) {
      _unsafe_vT_destructor();
    }
  }

  auto _checked_move_semantic(Result && other) -> Result& {
    _checked_invalidate();
    if (other.ok()) {
      _unsafe_vT_move_from(static_cast<T&&>(other._unsafe_vT_ref()));
      is_ok = true;
    } else {
      vR = other.vR;
      is_ok = false;
    }
    other._checked_invalidate();
    return *this;
  }

  auto _checked_copy_semantic(Result & other) -> Result& {
    _checked_vT_destructor();
    if (other.ok()) {
      _unsafe_vT_copy_from(other._unsafe_vT_ref());
      is_ok = true;
    } else {
      vR = other.vR;
      is_ok = false;
    }
    return *this;
  }

  consteval auto _meta_reason_has_invalidate() const noexcept -> bool {
    return requires { R::ResultInvalidated; };
  }

  auto _checked_invalidate() noexcept -> void {
    _checked_vT_destructor();
    is_ok = false;
    if constexpr (_meta_reason_has_invalidate()) {
      has_R = true;
      vR = R::ResultInvalidated;
    } else {
      has_R = false;
    }
  }

}; // struct Result

} // namespace mpp
