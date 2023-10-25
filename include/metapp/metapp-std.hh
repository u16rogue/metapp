#pragma once

#include <metapp/common.hh>
#include <metapp/concepts.hh>
#include <new>

//=========================================================================================
//=== Defer statement

#define mpp_lock(mutx) \
  const mpp::details::__mpp_scope_in mpp_glue(__mpp_lock_, __LINE__) = [&, lk = std::lock_guard<std::mutex>(mutx)]()


//=========================================================================================
//=== Results
namespace mpp {

namespace details::result {
  enum class Response {
    SUCCESS = 0,
    FAILURE = 1, // Unspecified failure
  };
} // details::result

/*
 *  Optional type with alternative value for lack of `T` presented by `R`
 */
template <typename T, typename R = details::result::Response, bool custom_arrow_operator_chaining = true>
struct Result {
  using ResponseType = R;
  static_assert(requires { R::SUCCESS; } && static_cast<int>(R::SUCCESS) == 0, "Invalid Response enumerator `S`. Must contain 'SUCCESS' and its value must be 0.");
  static_assert(requires { R::FAILURE; } && static_cast<int>(R::FAILURE) == 1, "Invalid Response enumerator `S`. Must contain 'FAILURE' and its value must be 1.");

  Result()
    : is_success(false), as_failure(R::FAILURE)
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
      other.as_failure = R::FAILURE;
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
  Result(R response)
    : is_success(false)
  {
    if (response == R::SUCCESS) {
      response = R::FAILURE;
    }
    as_failure = response;
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

  auto why() const noexcept -> R {
    if (!is_success) {
      return as_failure;
    } else {
      return R::SUCCESS;
    }
  }

private:
  bool is_success;
  union {
    R  as_failure;
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

} // namespace mpp
