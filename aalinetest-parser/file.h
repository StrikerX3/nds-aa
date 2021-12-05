#pragma once

#include "types.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <vector>

template <typename T>
inline std::vector<T> LoadBin(const std::filesystem::path &path) {
    std::basic_ifstream<T> file{path, std::ios::binary};
    return {std::istreambuf_iterator<T>{file}, {}};
}

struct membuf : std::streambuf {
    membuf(char *begin, char *end) {
        this->setg(begin, begin, end);
    }
};

inline std::unique_ptr<Data> readFile(std::filesystem::path path) {
    if (!std::filesystem::is_regular_file(path)) {
        std::cout << path.string() << " does not exist or is not a file.\n";
        return nullptr;
    }

    std::cout << "Loading " << path.string() << "... ";
    std::vector<char> buffer = LoadBin<char>(path);
    membuf mbuf(buffer.data(), buffer.data() + buffer.size());
    std::istream in{&mbuf};

    auto pData = std::make_unique<Data>();
    auto &lines = pData->lines;

    in.read((char *)&pData->type, 1);
    in.read((char *)&pData->minX, sizeof(pData->minX));
    in.read((char *)&pData->maxX, sizeof(pData->maxX));
    in.read((char *)&pData->minY, sizeof(pData->minY));
    in.read((char *)&pData->maxY, sizeof(pData->maxY));

    switch (pData->type) {
    case 0: std::cout << "Top left"; break;
    case 1: std::cout << "Bottom left"; break;
    case 2: std::cout << "Top right"; break;
    case 3: std::cout << "Bottom right"; break;
    default: std::cout << "Invalid type (" << (int)pData->type << ")"; return nullptr;
    }
    std::cout << ", " << pData->minX << "x" << (int)pData->minY << " to " << pData->maxX << "x" << (int)pData->maxY;

    auto readSpans = [&](int prevX, int prevY) {
        int startY = (pData->type != TEST_BOTTOM) ? 0 : std::min(prevY, 191);
        int endY = (pData->type != TEST_TOP) ? 191 : std::min(prevY, 191);
        for (int checkY = startY; checkY <= endY; checkY++) {
            for (;;) {
                u16 x;
                u8 y;
                in.read((char *)&x, sizeof(x));
                in.read((char *)&y, sizeof(y));
                if (x == 0xFFFF && y == 0xFF) {
                    break;
                }
                char entry;
                in.read(&entry, 1);
                while ((uint8_t)entry != 0xFF) {
                    lines[prevY][prevX].spans[y][x++] = entry;
                    in.read(&entry, 1);
                }
            }
        }
    };

    u16 coordX;
    u8 coordY;
    int prevX = 0;
    int prevY = 0;
    for (int y = pData->minY; y <= pData->maxY; y++) {
        for (int x = pData->minX; x <= pData->maxX; x++) {
            in.read((char *)&coordX, sizeof(coordX));
            in.read((char *)&coordY, sizeof(coordY));
            if (coordX != (u16)prevX || coordY != (u8)prevY) {
                std::cout << " -- Invalid file\n";
                return nullptr;
            }
            readSpans(prevX, prevY);
            prevX = x;
            prevY = y;
        }
    }

    readSpans(prevX, prevY);

    std::cout << " -- OK\n";

    return std::move(pData);
}
