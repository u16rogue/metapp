
namespace mpp {

template <typename T>
concept ConstraintHasArrowOperator = requires (T a) {
  a.operator->();
};

} // namespace mpp
