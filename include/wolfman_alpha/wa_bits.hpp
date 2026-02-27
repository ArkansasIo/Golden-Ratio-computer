#pragma once
#include "wa_types.hpp"
#include <stdexcept>

namespace wa {

class BitArray {
public:
  explicit BitArray(std::size_t bitCount = 0) { resize(bitCount); }

  void resize(std::size_t bitCount) {
    bits_ = bitCount;
    data_.assign((bitCount + 7) / 8, 0);
  }

  std::size_t size() const { return bits_; }

  u8 get(std::size_t i) const {
    if (i >= bits_) throw std::out_of_range("BitArray::get");
    return (data_[i >> 3] >> (i & 7)) & 1u;
  }

  void set(std::size_t i, u8 v) {
    if (i >= bits_) throw std::out_of_range("BitArray::set");
    const u8 mask = static_cast<u8>(1u << (i & 7));
    if (v & 1u) data_[i >> 3] |= mask;
    else        data_[i >> 3] &= static_cast<u8>(~mask);
  }

  void flip(std::size_t i) { set(i, static_cast<u8>(get(i) ^ 1u)); }

private:
  std::size_t bits_{0};
  std::vector<u8> data_;
};

} // namespace wa
