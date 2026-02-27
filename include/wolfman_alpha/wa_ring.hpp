#pragma once
#include "wa_types.hpp"
#include "wa_gear.hpp"
#include <stdexcept>
#include <vector>

namespace wa {

class Ring {
public:
  Ring(int gearCount = 360, int defaultTeeth = 20, double defaultPitchRadius = 10.0)
    : dir_(Dir::Right), offset_(0) {
    gears_.resize(gearCount);
    for (auto& g : gears_) {
      g.teeth = defaultTeeth;
      g.pitchRadius = defaultPitchRadius;
      g.bit = 0;
    }
  }

  int gearCount() const { return static_cast<int>(gears_.size()); }
  Dir dir() const { return dir_; }
  int offset() const { return offset_; }

  void shift(Dir d, int k = 1) {
    const int n = gearCount();
    if (n <= 0) return;
    k %= n; if (k < 0) k += n;

    // Convention: RIGHT => offset decreases, LEFT => offset increases
    if (d == Dir::Right) offset_ = (offset_ - k) % n;
    else                offset_ = (offset_ + k) % n;

    if (offset_ < 0) offset_ += n;
    dir_ = d;
  }

  void tick(int k = 1) { shift(dir_, k); }

  int mapIndex(int logicalIndex) const {
    const int n = gearCount();
    if (logicalIndex < 0 || logicalIndex >= n) throw std::out_of_range("Ring gear index");
    int idx = (logicalIndex + offset_) % n;
    if (idx < 0) idx += n;
    return idx;
  }

  u8  getBit(int logicalIndex) const { return gears_[mapIndex(logicalIndex)].bit & 1u; }
  void setBit(int logicalIndex, u8 v) { gears_[mapIndex(logicalIndex)].bit = (v & 1u); }
  void flipBit(int logicalIndex) { setBit(logicalIndex, static_cast<u8>(getBit(logicalIndex) ^ 1u)); }

  const Gear& gearAtLogical(int logicalIndex) const { return gears_[mapIndex(logicalIndex)]; }
  Gear&       gearAtLogical(int logicalIndex)       { return gears_[mapIndex(logicalIndex)]; }

private:
  std::vector<Gear> gears_;
  Dir dir_;
  int offset_;
};

} // namespace wa
