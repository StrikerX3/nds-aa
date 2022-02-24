#pragma once

#include <array>
#include <cstdint>
#include <iterator>
#include <span>
#include <unordered_map>
#include <vector>

constexpr int TEST_TOP = 0;
constexpr int TEST_BOTTOM = 1;
constexpr int TEST_LEFT = 2;
constexpr int TEST_RIGHT = 3;

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using i32 = int32_t;

struct Line {
    struct Span {
        size_t vecOffset;
        u16 length;
        u16 x0;
    };
    std::vector<u8> data;
    std::unordered_map<u8, std::vector<Span>> spans; // key is Y

    void Add(u16 x, u8 y, std::span<u8> values) {
        spans[y].push_back({.vecOffset = data.size(), .length = (u16)values.size(), .x0 = x});
        std::copy(values.begin(), values.end(), std::back_inserter(data));
    }

    u8 Pixel(u16 x, u8 y) const {
        if (!spans.contains(y)) {
            return 0;
        }

        for (auto &span : spans.at(y)) {
            if (x < span.x0) {
                continue;
            }

            if ((size_t)x - span.x0 >= (size_t)span.length) {
                continue;
            }

            return data[span.vecOffset + x - span.x0];
        }
        return 0;
    }

    bool ContainsY(u8 y) const {
        return spans.contains(y);
    }
};

struct Data {
    u8 type;
    u16 minX, maxX;
    u8 minY, maxY;
    std::array<std::array<Line, 256 + 1>, 192 + 1> lines;
};
