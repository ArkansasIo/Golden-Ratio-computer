#pragma once
#include "wa_ring.hpp"
#include <sstream>
#include <iomanip>
#include <vector>
#include <stdexcept>

namespace wa {

class Machine {
public:
  Machine(int rings = 10, int gearsPerRing = 360) {
    rings_.reserve(rings);
    for (int r = 0; r < rings; r++) rings_.emplace_back(gearsPerRing);
  }

  int ringCount() const { return static_cast<int>(rings_.size()); }
  int gearsPerRing() const { return rings_.empty() ? 0 : rings_[0].gearCount(); }

  Ring& ring(int r) { checkRing(r); return rings_[r]; }
  const Ring& ring(int r) const { checkRing(r); return rings_[r]; }

  u8 getBit(int r, int i) const { return ring(r).getBit(i); }
  void setBit(int r, int i, u8 v) { ring(r).setBit(i, v); }
  void flipBit(int r, int i) { ring(r).flipBit(i); }

  void shiftRing(int r, Dir d, int k) { ring(r).shift(d, k); }
  void tickAll(int k = 1) { for (auto& rg : rings_) rg.tick(k); }

  std::string capacityString(bool twoBitsPerGearCell) const {
    const long long bits = static_cast<long long>(ringCount()) * static_cast<long long>(gearsPerRing()) * (twoBitsPerGearCell ? 2LL : 1LL);
    const double log10approx = bits * 0.30102999566;
    std::ostringstream oss;
    oss << "Capacity: 2^" << bits << " integers (≈ 10^" << std::fixed << std::setprecision(2) << log10approx << ")";
    oss << " ; range = [0 .. 2^" << bits << " - 1]";
    return oss.str();
  }

  std::string dumpRing(int r, int count = 64) const {
    const auto& rg = ring(r);
    const int n = rg.gearCount();
    const int c = (count > n) ? n : count;
    std::ostringstream oss;
    oss << "Ring " << r << " dir=" << to_string(rg.dir()) << " offset=" << rg.offset() << " bits:";
    for (int i = 0; i < c; i++) oss << int(rg.getBit(i));
    return oss.str();
  }

private:
  std::vector<Ring> rings_;

  void checkRing(int r) const {
    if (r < 0 || r >= (int)rings_.size()) throw std::out_of_range("Ring out of range");
  }
};

} // namespace wa
