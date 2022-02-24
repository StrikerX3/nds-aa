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
        u8 length;
        u8 x0;
    };
    std::vector<u8> data;
    std::unordered_map<u8, std::vector<Span>> spans; // key is Y

    void Add(u8 x, u8 y, std::span<u8> values) {
        spans[y].push_back({.vecOffset = spans.size(), .length = (u8)(values.size() - 1), .x0 = x});
        std::copy(values.begin(), values.end(), std::back_inserter(data));
    }

    u8 Pixel(u8 x, u8 y) const {
        if (!spans.contains(y)) {
            return 0;
        }

        for (auto &span : spans.at(y)) {
            if (x < span.x0) {
                continue;
            }

            x -= span.x0;
            if (x > span.length) {
                continue;
            }

            return data[span.vecOffset + x];
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
