#pragma once
#include <boost/variant/static_visitor.hpp>

namespace admplug {
  class ElementComparator : public boost::static_visitor<bool> {
  public:
      template<typename T>
      bool operator()(T const& lhs, T const& rhs) const {
          return lhs == rhs;
      }

      template<typename T, typename U>
      bool operator()(T const& lhs, U const& rhs) const {
          return false;
      }
  };
}
