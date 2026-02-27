#pragma once
#include "wa_word.hpp"

namespace wa {

inline void alu_xor(Machine& m, const WordRef& a, const WordRef& b, const WordRef& out) {
  const int n = word_bits(a.size);
  for (int i=0;i<n;i++) set_word_bit(m, out, i, (u8)(get_word_bit(m,a,i) ^ get_word_bit(m,b,i)));
}

inline void alu_and(Machine& m, const WordRef& a, const WordRef& b, const WordRef& out) {
  const int n = word_bits(a.size);
  for (int i=0;i<n;i++) set_word_bit(m, out, i, (u8)(get_word_bit(m,a,i) & get_word_bit(m,b,i)));
}

inline void alu_or(Machine& m, const WordRef& a, const WordRef& b, const WordRef& out) {
  const int n = word_bits(a.size);
  for (int i=0;i<n;i++) set_word_bit(m, out, i, (u8)(get_word_bit(m,a,i) | get_word_bit(m,b,i)));
}

inline u8 alu_add(Machine& m, const WordRef& a, const WordRef& b, const WordRef& out) {
  const int n = word_bits(a.size);
  u8 carry = 0;
  for (int i=0;i<n;i++) {
    u8 A = get_word_bit(m,a,i), B = get_word_bit(m,b,i);
    u8 sum = (u8)(A ^ B ^ carry);
    carry  = (u8)((A & B) | (A & carry) | (B & carry));
    set_word_bit(m,out,i,sum);
  }
  return carry;
}

} // namespace wa
