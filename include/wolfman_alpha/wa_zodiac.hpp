#pragma once
#include "wa_types.hpp"
#include <array>
#include <string_view>

namespace wa {

enum class Zodiac13 : u8 { Z0=0, Z1, Z2, Z3, Z4, Z5, Z6, Z7, Z8, Z9, Z10, Z11, Z12 };
inline constexpr int ZODIAC_COUNT = 13;

inline constexpr std::array<std::string_view, ZODIAC_COUNT> ZodiacNames = {
  "Z0","Z1","Z2","Z3","Z4","Z5","Z6","Z7","Z8","Z9","Z10","Z11","Z12"
};

inline Zodiac13 glyphAtIndex(int gearIndex, int gearCount=360) {
  if (gearCount <= 0) return Zodiac13::Z0;
  if (gearIndex < 0) gearIndex = 0;
  if (gearIndex >= gearCount) gearIndex = gearCount - 1;
  int g = (gearIndex * ZODIAC_COUNT) / gearCount; // 0..12
  if (g < 0) g = 0;
  if (g >= ZODIAC_COUNT) g = ZODIAC_COUNT - 1;
  return (Zodiac13)g;
}

inline Zodiac13 activeGlyphFromOffset(int offset, int gearCount=360) {
  int idx = offset % gearCount; if (idx < 0) idx += gearCount;
  return glyphAtIndex(idx, gearCount);
}

} // namespace wa
