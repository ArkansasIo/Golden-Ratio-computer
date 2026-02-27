#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <array>

namespace wa {

using u8  = std::uint8_t;
using u16 = std::uint16_t;
using u32 = std::uint32_t;
using u64 = std::uint64_t;
using i32 = std::int32_t;

struct Vec2 { double x{0}, y{0}; };

enum class Dir : u8 { Left = 0, Right = 1 };

inline const char* to_string(Dir d) { return (d == Dir::Left) ? "LEFT" : "RIGHT"; }

} // namespace wa
