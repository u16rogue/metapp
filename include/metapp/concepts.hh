
namespace mpp {

    template <typename T>
    concept ConstraintHasArrowOperator = requires (T a) {
        a.operator->();
    };

    template <typename T>
    concept ConstraintEnumHasInvalidated = requires { T::Invalidated; };

    template <typename Fn, typename T>
    concept ConstraintAcceptsArgOf = requires (Fn fn, T v) { fn(v); };

} // namespace mpp
