#pragma once

#include <metapp/common.hh>
#include <metapp/concepts.hh>

#define mpp_lock(mutx) \
    const mpp::details::__mpp_scope_in mpp_glue(__mpp_lock_, __LINE__) = [&, lk = std::lock_guard<std::mutex>(mutx)]()

namespace mpp {
    namespace details::result {
        enum class Reason : mpp::u8 {
            Invalidated,
            Unspecified,
        };
    } // mpp::details::result

    template <typename T> struct Err {
        T error;
    };

    /// Optional type with alternative value for lack of `Ok` represented by `Err`
    /// Remarks:
    ///     * If moving from <= v0.9.x see `BREAKING 0.10.0`
    ///
    ///     * Treat destructed `Results` as destructed Ok/Err.
    ///
    ///     * If `Ok` has a custom `->` defined `Result` will automatically chain its own `->` to `Ok`'s.
    ///     You can disable this by setting `custom_arrow_operator_chaining` to false on its template parameter.
    ///     What this does is in the case of `Result<std::unique_ptr<foo>> bar;` doing `bar->` will result to
    ///     `foo` instead of `std::unique_ptr<foo>`.
    ///
    ///     * If the compiler gets confused due to ambiguous instantiation
    ///     you can wrap your error in a unique struct and initialize an error
    ///     state using that struct to disambiguate. A utility `Err<T>` is provided
    ///     in the same namespace and can be used as such: `Err<int> { 1234 }`
    ///
    ///     * No direct constructor is provided for `Err`. This is intentional.
    ///
    /// Internal Remarks:
    ///     * BREAKING 0.10.0: `Result` is no longer treated as an object itself it is now just
    ///     a wrapper to `Ok` and `Err`. Previously this was done to truly make `Result`
    ///     a valid only state where if there was an `Ok` that `Ok` is always valid, this
    ///     resulted into making `Result` have 3 states to make sure that if we have a value
    ///     of `Ok` that `Ok` is also valid (eg a `unique_ptr` has a valid pointer) which results
    ///     to enforcing state management when doing semantics such as marking a moved `Result` as
    ///     invalidated now this has changed to either we have an `Ok` value which makes `Result`
    ///     a true state or an `Err` value which makes `Result` a false state.
    ///
    ///     * The `invalidated` state has been removed as this should be handled
    ///     by the underlying types of `Ok` and `Err`. Now when moving an underlying
    ///     type that has no copy constructor the `Result` will remain valid. Destructor 
    ///     operations will only destruct the underlying type and will not destruct `Result`
    ///     therefore any subsequent operation will now operate on the destructed underlying type.
    ///     This is a very important distinction as a `Result<std::unique_ptr<T>>` that was destructed will
    ///     still have an `Ok` state but the underlying type has already been destroyed causing access to `T`
    ///     an illegal behavior. TREAT DESTRUCTED RESULTS AS DESTRUCTED OK/ERR.
    ///     Todo:
    ///         * Support type `void`
    template <typename Ok, typename Err = details::result::Reason, bool custom_arrow_operator_chaining = true>
    struct Result {

        using OkType = Ok;
        using ErrType = Err;

        //--------------------------------------------------------------------------------------------------

        /// Direct `Ok` constructor
        template <typename... VArgs>
        constexpr Result(VArgs &&... args)
            : has_ok(true)
        {
            _unsafe_valOkConstruct(static_cast<VArgs>(args)...);
        }

        /// Move `Ok` constructor
        constexpr Result(Ok && value)
            : has_ok(true)
        {
            _unsafe_valOkMoveFrom(static_cast<Ok&&>(value));
        }

        /// Copy `Ok` constructor
        constexpr Result(Ok & value)
            : has_ok(true)
        {
            _unsafe_valOkCopyFrom(value);
        }

        //--------------------------------------------------------------------------------------------------

        /// Move `Err` constructor
        constexpr Result(Err && value)
            : has_ok(false)
        {
            _unsafe_valErrMoveFrom(static_cast<Err&&>(value));
        }

        /// Copy `Err` constructor
        constexpr Result(Err & value)
            : has_ok(false)
        {
            _unsafe_valErrCopyFrom(value);
        }

        //--------------------------------------------------------------------------------------------------

        /// Move semantics
        constexpr Result(Result && other)
        {
            doMoveSemanticChecked(false, static_cast<Result&&>(other));
        }

        [[maybe_unused]] constexpr auto operator=(Result && other) -> Result& {
            return doMoveSemanticChecked(true, static_cast<Result&&>(other));
        }

        /// Copy semantics
        constexpr Result(Result & other)
            : has_ok(false)
        {
            doCopySemanticChecked(false, other);
        }

        [[maybe_unused]] constexpr auto operator=(Result & other) -> Result& {
            return doCopySemanticChecked(true, other);
        }

        constexpr ~Result() {
            valAnyDestructChecked();
        }

        //--------------------------------------------------------------------------------------------------

        constexpr auto hasOk() const noexcept -> bool {
            return has_ok;
        }

        constexpr auto hasErr() const noexcept -> bool {
            return !hasOk();
        }

        constexpr explicit operator bool() const noexcept {
            return hasOk();
        }

        //--------------------------------------------------------------------------------------------------

        constexpr auto operator->() -> Ok& requires (ConstraintHasArrowOperator<Ok> && custom_arrow_operator_chaining) {
            // TODO: refactor this to default construct T when !has_value
            return _unsafe_valOkRef();
        }

        constexpr auto operator*() -> Ok& {
            // TODO: refactor this to default construct T when !has_value
            return _unsafe_valOkRef();
        }

        constexpr auto operator->() -> Ok* {
            // TODO: refactor this to default construct T when !has_value
            return &_unsafe_valOkRef();
        }

        //--------------------------------------------------------------------------------------------------

        [[maybe_unused]] constexpr auto valOkAccess(const auto && callback) /*const*/ noexcept -> Result& {
            return valOkAccess(callback, []{});
        }

        [[maybe_unused]] constexpr auto valOkAccess(const auto && callback, const auto && fallback) /*const*/ noexcept -> Result& {
            if (hasOk()) {
                if constexpr (ConstraintAcceptsArgOf<decltype(callback), Ok>) {
                    callback(_unsafe_valOkRef());
                } else {
                    callback();
                }
            } else {
                if constexpr (ConstraintAcceptsArgOf<decltype(fallback), Err>) {
                    fallback(_unsafe_valErrRef());
                } else {
                    fallback();
                }
            }
            return *this;
        }

        template <typename Transact>
        [[nodiscard]] constexpr auto valOkTryTransact(const auto && callback, const auto && fallback) /*const*/ noexcept -> Transact {
            if (hasOk()) {
                if constexpr (ConstraintAcceptsArgOf<decltype(callback), Ok>) {
                    return callback(_unsafe_valOkRef());
                } else {
                    return callback();
                }
            } else {
                if constexpr (ConstraintAcceptsArgOf<decltype(fallback), Err>) {
                    return fallback(_unsafe_valErrRef());
                } else {
                    return fallback();
                }
            }
        }

        //--------------------------------------------------------------------------------------------------

        [[maybe_unused]] constexpr auto valErrAccess(const auto && callback) /*const*/ noexcept -> Result& {
            return valErrAccess(callback, []{});
        }

        [[maybe_unused]] constexpr auto valErrAccess(const auto && callback, const auto && fallback) /*const*/ noexcept -> Result& {
            return valOkAccess(fallback, callback);
        }

        template <typename Transact>
        [[nodiscard]] constexpr auto valErrTryTransact(const auto && callback, const auto && fallback) /*const*/ noexcept -> Transact {
            return valOkTryTransact<Transact>(fallback, callback);
        }

        //--------------------------------------------------------------------------------------------------
    private:
        bool has_ok;
        union {
            u8  _raw_ok[sizeof(Ok)];
            Err _raw_err;
        };
        static_assert(sizeof(_raw_ok) == sizeof(Ok), "Pseudo container for type `T` isn't properly aligned.");

    private:

        //--------------------------------------------------------------------------------------------------

        auto _unsafe_valOkRef() -> Ok& {
            return *reinterpret_cast<Ok*>(&_raw_ok);
        }

        auto _unsafe_valOkRef() const -> const Ok& {
            return *reinterpret_cast<const Ok*>(&_raw_ok);
        }

        template <typename... VArgs>
        auto _unsafe_valOkConstruct(VArgs &&... args) {
            new (&_unsafe_valOkRef()) Ok(static_cast<VArgs&&>(args)...);
        }

        auto _unsafe_valOkDestruct() -> void {
            _unsafe_valOkRef().~Ok();
        }

        auto _unsafe_valOkMoveFrom(Ok && other) -> void {
            new (&_unsafe_valOkRef()) Ok(static_cast<Ok&&>(other));
        }

        auto _unsafe_valOkCopyFrom(Ok & other) -> void {
            new (&_unsafe_valOkRef()) Ok(other);
        }

        auto valOkDestructChecked() -> void {
            if (hasOk()) {
                _unsafe_valOkDestruct();
            }
        }

        //--------------------------------------------------------------------------------------------------

        auto _unsafe_valErrRef() -> Err& {
            return _raw_err;
        }

        auto _unsafe_valErrRef() const -> Err& {
            return *reinterpret_cast<const Err*>(&_raw_err);
        }

        template <typename... VArgs>
        auto _unsafe_valErrConstruct(VArgs &&... args) {
            new (&_unsafe_valErrRef()) Err(static_cast<VArgs&&>(args)...);
        }

        auto _unsafe_valErrDestruct() -> void {
            _unsafe_valErrRef().~Err();
        }

        auto _unsafe_valErrMoveFrom(Err && other) -> void {
            new (&_unsafe_valErrRef()) Err(static_cast<Err&&>(other));
        }

        auto _unsafe_valErrCopyFrom(Err & other) -> void {
            new (&_unsafe_valErrRef()) Err(other);
        }

        auto valErrDestructChecked() -> void {
            if (hasErr()) {
                _unsafe_valErrDestruct();
            }
        }

        //--------------------------------------------------------------------------------------------------

        auto valAnyDestructChecked() noexcept -> void {
            if (hasOk()) {
                valOkDestructChecked();
            } else {
                valErrDestructChecked();
            }
        }

        [[maybe_unused]] auto doMoveSemanticChecked(bool destruct_self, Result && other) -> Result& {
            if (destruct_self) valAnyDestructChecked();

            if (other.hasOk()) {
                _unsafe_valOkMoveFrom(static_cast<Ok&&>(other._unsafe_valOkRef()));
            } else {
                _unsafe_valErrMoveFrom(static_cast<Err&&>(other._unsafe_valErrRef()));
            }

            has_ok = other.has_ok;
            return *this;
        }

        [[maybe_unused]] auto doCopySemanticChecked(bool destruct_self, Result & other) -> Result& {
            if (destruct_self) valAnyDestructChecked();

            if (other.hasOk()) {
                _unsafe_valOkCopyFrom(other._unsafe_valOkRef());
            } else {
                _unsafe_valErrCopyFrom(other._unsafe_valErrRef());
            }

            has_ok = other.has_ok;
            return *this;
        }
    }; // struct Result

    /// Optional type with alternative value for lack of `Ok` represented by `Err` where `Ok` will always
    /// be the state it was initialized in, otherwise invalidated.
    ///
    /// Eg. If `Ok` with type `unique_ptr` was initialized with a valid pointer rest assured that operations to
    /// `Ok` will always result to the same valid `unique_ptr` as such moving `Result` and its underlying `Ok`
    /// will result to `Result` being marked as invalidated of `Err`.
    ///
    /// Remarks:
    ///     * To be implemented
    ///     * This implements the original behavior of <= 0.9.x `Result`
    template <typename Ok, typename Err = details::result::Reason, Err invalidated = details::result::Reason::Invalidated, bool custom_arrow_operator_chaining = true>
    struct InvalidatingResult : Result<Ok, Err, custom_arrow_operator_chaining> {
        // TODO: implement
    };

} // namespace mpp
