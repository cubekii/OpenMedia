#pragma once

#include <openmedia/media.h>

namespace openmedia {

struct OPENMEDIA_ABI Rational {
  int32_t num = 0;
  int32_t den = 0;

  constexpr auto toDouble() const noexcept -> double {
    return den != 0 ? static_cast<double>(num) / static_cast<double>(den) : 0.0;
  }

  constexpr auto toFloat() const noexcept -> double {
    return den != 0 ? static_cast<float>(num) / static_cast<float>(den) : 0.0f;
  }
};

constexpr auto operator==(const Rational& lhs, const Rational& rhs) noexcept -> bool {
  return lhs.num == rhs.num && lhs.den == rhs.den;
}

} // namespace openmedia
