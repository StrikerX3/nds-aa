#pragma once

#include <array>
#include <cstdint>
#include <unordered_map>

constexpr int TEST_TOP = 0;
constexpr int TEST_BOTTOM = 1;
constexpr int TEST_LEFT = 2;
constexpr int TEST_RIGHT = 3;

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using i32 = int32_t;

struct Line {
    std::unordered_map<u8, std::array<u8, 256>> spans;
};

struct Data {
    u8 type;
    u16 minX, maxX;
    u8 minY, maxY;
    std::array<std::array<Line, 256 + 1>, 192 + 1> lines;
};
