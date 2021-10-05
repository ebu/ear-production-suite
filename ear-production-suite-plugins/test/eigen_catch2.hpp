#pragma once
#include <Eigen/Dense>
#include <boost/optional.hpp>
#include <catch2/catch_all.hpp>

const Eigen::IOFormat HeavyFmt(Eigen::FullPrecision, 0, ", ", ";\n", "[", "]",
                               "[", "]");

// improve the formatting of Eigen matrices
namespace Catch {
template <typename T>
struct StringMaker<Eigen::DenseBase<T>> {
  static std::string convert(Eigen::DenseBase<T> const& value) {
    std::ostringstream ss;
    ss << value.format(HeavyFmt);
    return ss.str();
  }
};
}  // namespace Catch

template <typename T>
class EigenIsApprox : public Catch::Matchers::MatcherBase<T> {
  T value;
  boost::optional<typename T::Scalar> precision;

 public:
  EigenIsApprox(T value, boost::optional<typename T::Scalar> precision)
      : value(std::move(value)), precision(precision) {}

  bool match(T const& other) const override {
    if (precision)
      return value.isApprox(other, precision.get());
    else
      return value.isApprox(other);
  }

  virtual std::string describe() const override {
    std::ostringstream ss;
    ss << std::endl
       << "    is not approximately equal to" << std::endl
       << value.format(HeavyFmt);
    if (precision) ss << std::endl << "    with precision " << precision.get();
    return ss.str();
  }
};

/// check that two Eigen Matrices are approximately equal using value.isApprox
template <typename T>
inline EigenIsApprox<T> IsApprox(
    T value, boost::optional<typename T::Scalar> precision = boost::none) {
  return EigenIsApprox<T>(std::move(value), precision);
}
