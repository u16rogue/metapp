
namespace mpp {

template <typename T>
concept ConstraintHasArrowOperator = requires (T a) {
  a.operator->();
};

template <typename T>
concept ConstraintEnumHasInvalidated = requires { T::Invalidated; };

} // namespace mpp
