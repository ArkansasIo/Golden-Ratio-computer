#pragma once
#include "wa_machine.hpp"
#include <stdexcept>

namespace wa {

enum class WordSize { W64 = 64, W360 = 360, W720 = 720 };

struct WordRef {
  WordSize size;
  int ringA;
  int ringB;
  int baseIndex;

  static WordRef w64(int ring, int base=0)   { return {WordSize::W64,  ring, -1, base}; }
  static WordRef w360(int ring)              { return {WordSize::W360, ring, -1, 0}; }
  static WordRef w720(int ring0, int ring1)  { return {WordSize::W720, ring0, ring1, 0}; }
};

inline int word_bits(WordSize ws) { return static_cast<int>(ws); }

inline u8 get_word_bit(const Machine& m, const WordRef& w, int bit) {
  const int n = word_bits(w.size);
  if (bit < 0 || bit >= n) throw std::out_of_range("get_word_bit");
  if (w.size == WordSize::W64)  return m.getBit(w.ringA, w.baseIndex + bit);
  if (w.size == WordSize::W360) return m.getBit(w.ringA, bit);
  return (bit < 360) ? m.getBit(w.ringA, bit) : m.getBit(w.ringB, bit - 360);
}

inline void set_word_bit(Machine& m, const WordRef& w, int bit, u8 v) {
  const int n = word_bits(w.size);
  if (bit < 0 || bit >= n) throw std::out_of_range("set_word_bit");
  v &= 1u;
  if (w.size == WordSize::W64)  { m.setBit(w.ringA, w.baseIndex + bit, v); return; }
  if (w.size == WordSize::W360) { m.setBit(w.ringA, bit, v); return; }
  if (bit < 360) m.setBit(w.ringA, bit, v);
  else           m.setBit(w.ringB, bit - 360, v);
}

} // namespace wa
