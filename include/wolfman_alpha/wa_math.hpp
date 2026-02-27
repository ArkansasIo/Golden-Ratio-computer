#pragma once
#include "wa_types.hpp"
#include <cmath>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <vector>

namespace wa::math {

constexpr double PI = 3.14159265358979323846;

inline double deg_to_rad(double deg) { return deg * PI / 180.0; }
inline double rad_to_deg(double rad) { return rad * 180.0 / PI; }

inline double ring_radius_from_pitch_radius(double r, int N) {
  return r / std::sin(PI / static_cast<double>(N));
}

inline Vec2 polar(double R, double theta_rad) {
  return { R * std::cos(theta_rad), R * std::sin(theta_rad) };
}

inline int normalize_mod(int value, int mod) {
  if (mod <= 0) throw std::invalid_argument("mod must be > 0");
  int out = value % mod;
  if (out < 0) out += mod;
  return out;
}

inline int add_mod(int a, int b, int mod) {
  return normalize_mod(normalize_mod(a, mod) + normalize_mod(b, mod), mod);
}

inline int sub_mod(int a, int b, int mod) {
  return normalize_mod(normalize_mod(a, mod) - normalize_mod(b, mod), mod);
}

inline int mul_mod(int a, int b, int mod) {
  const long long aa = normalize_mod(a, mod);
  const long long bb = normalize_mod(b, mod);
  return static_cast<int>((aa * bb) % mod);
}

inline int pow_mod(int base, int exp, int mod) {
  if (exp < 0) throw std::invalid_argument("exp must be >= 0");
  int result = 1 % mod;
  int b = normalize_mod(base, mod);
  int e = exp;
  while (e > 0) {
    if (e & 1) result = mul_mod(result, b, mod);
    b = mul_mod(b, b, mod);
    e >>= 1;
  }
  return result;
}

inline int gcd(int a, int b) {
  int x = (a < 0) ? -a : a;
  int y = (b < 0) ? -b : b;
  while (y != 0) {
    const int t = x % y;
    x = y;
    y = t;
  }
  return x;
}

inline int lcm(int a, int b) {
  if (a == 0 || b == 0) return 0;
  return (a / gcd(a, b)) * b;
}

inline double ring_index_to_angle_deg(int index, int gearCount) {
  if (gearCount <= 0) throw std::invalid_argument("gearCount must be > 0");
  const double step = 360.0 / static_cast<double>(gearCount);
  return static_cast<double>(normalize_mod(index, gearCount)) * step;
}

inline double ring_index_to_angle_rad(int index, int gearCount) {
  if (gearCount <= 0) throw std::invalid_argument("gearCount must be > 0");
  const double step = (2.0 * PI) / static_cast<double>(gearCount);
  return static_cast<double>(normalize_mod(index, gearCount)) * step;
}

inline int angle_rad_to_ring_index(double angleRad, int gearCount) {
  if (gearCount <= 0) throw std::invalid_argument("gearCount must be > 0");
  const double wrapped = std::fmod(angleRad, 2.0 * PI);
  const double angle = (wrapped < 0.0) ? wrapped + (2.0 * PI) : wrapped;
  const double ratio = angle / (2.0 * PI);
  const int idx = static_cast<int>(std::floor(ratio * static_cast<double>(gearCount)));
  return normalize_mod(idx, gearCount);
}

inline int logical_to_physical_index(int logicalIndex, int offset, int gearCount) {
  if (gearCount <= 0) throw std::invalid_argument("gearCount must be > 0");
  return normalize_mod(logicalIndex + offset, gearCount);
}

inline int physical_to_logical_index(int physicalIndex, int offset, int gearCount) {
  if (gearCount <= 0) throw std::invalid_argument("gearCount must be > 0");
  return normalize_mod(physicalIndex - offset, gearCount);
}

inline int apply_shift_offset(int offset, Dir d, int steps, int gearCount) {
  if (gearCount <= 0) throw std::invalid_argument("gearCount must be > 0");
  const int k = normalize_mod(steps, gearCount);
  if (d == Dir::Right) return normalize_mod(offset - k, gearCount);
  return normalize_mod(offset + k, gearCount);
}

inline int shortest_signed_steps(int fromIndex, int toIndex, int gearCount) {
  if (gearCount <= 0) throw std::invalid_argument("gearCount must be > 0");
  const int from = normalize_mod(fromIndex, gearCount);
  const int to = normalize_mod(toIndex, gearCount);
  const int cw = normalize_mod(from - to, gearCount);
  const int ccw = normalize_mod(to - from, gearCount);
  return (ccw <= cw) ? ccw : -cw;
}

inline std::vector<u8> xor_bits(const std::vector<u8>& a, const std::vector<u8>& b) {
  if (a.size() != b.size()) throw std::invalid_argument("xor_bits size mismatch");
  std::vector<u8> out(a.size(), 0);
  for (std::size_t i = 0; i < a.size(); ++i) out[i] = static_cast<u8>((a[i] ^ b[i]) & 1u);
  return out;
}

inline std::vector<u8> and_bits(const std::vector<u8>& a, const std::vector<u8>& b) {
  if (a.size() != b.size()) throw std::invalid_argument("and_bits size mismatch");
  std::vector<u8> out(a.size(), 0);
  for (std::size_t i = 0; i < a.size(); ++i) out[i] = static_cast<u8>((a[i] & b[i]) & 1u);
  return out;
}

inline std::vector<u8> or_bits(const std::vector<u8>& a, const std::vector<u8>& b) {
  if (a.size() != b.size()) throw std::invalid_argument("or_bits size mismatch");
  std::vector<u8> out(a.size(), 0);
  for (std::size_t i = 0; i < a.size(); ++i) out[i] = static_cast<u8>((a[i] | b[i]) & 1u);
  return out;
}

inline std::vector<u8> not_bits(const std::vector<u8>& a) {
  std::vector<u8> out(a.size(), 0);
  for (std::size_t i = 0; i < a.size(); ++i) out[i] = static_cast<u8>((a[i] ^ 1u) & 1u);
  return out;
}

inline std::vector<u8> rotate_bits_left(const std::vector<u8>& bits, int steps) {
  if (bits.empty()) return bits;
  const int n = static_cast<int>(bits.size());
  const int k = normalize_mod(steps, n);
  std::vector<u8> out(bits.size(), 0);
  for (int i = 0; i < n; ++i) out[i] = bits[normalize_mod(i + k, n)] & 1u;
  return out;
}

inline std::vector<u8> rotate_bits_right(const std::vector<u8>& bits, int steps) {
  return rotate_bits_left(bits, -steps);
}

inline u64 bits_to_u64(const std::vector<u8>& bits, std::size_t maxBits = 64) {
  const std::size_t n = (bits.size() < maxBits) ? bits.size() : maxBits;
  u64 out = 0;
  for (std::size_t i = 0; i < n; ++i) {
    out |= (static_cast<u64>(bits[i] & 1u) << i);
  }
  return out;
}

inline std::vector<u8> u64_to_bits(u64 value, std::size_t width) {
  std::vector<u8> out(width, 0);
  for (std::size_t i = 0; i < width; ++i) {
    out[i] = static_cast<u8>((value >> i) & 1ull);
  }
  return out;
}

inline std::string bits_to_string(const std::vector<u8>& bits) {
  std::string out;
  out.reserve(bits.size());
  for (u8 b : bits) out.push_back((b & 1u) ? '1' : '0');
  return out;
}

} // namespace wa::math
