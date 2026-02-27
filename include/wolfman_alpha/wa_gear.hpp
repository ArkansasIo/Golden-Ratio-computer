#pragma once
#include "wa_types.hpp"

namespace wa {

struct Gear {
  // 0=CCW, 1=CW (binary state)
  u8 bit{0};
  // Optional geometry metadata
  int teeth{20};
  double pitchRadius{10.0};
};

} // namespace wa
