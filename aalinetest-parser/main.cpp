#include <array>
#include <bitset>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <unordered_map>
#include <vector>

#include "slope.h"

using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using i32 = int32_t;

constexpr int TEST_TOP = 0;
constexpr int TEST_BOTTOM = 1;
constexpr int TEST_LEFT = 2;
constexpr int TEST_RIGHT = 3;

template <typename T>
std::vector<T> LoadBin(const std::filesystem::path &path) {
    std::basic_ifstream<T> file{path, std::ios::binary};
    return {std::istreambuf_iterator<T>{file}, {}};
}

struct membuf : std::streambuf {
    membuf(char *begin, char *end) {
        this->setg(begin, begin, end);
    }
};

struct Line {
    std::unordered_map<u8, std::array<u8, 256>> spans;
};

struct Data {
    u8 type;
    u16 minX, maxX;
    u8 minY, maxY;
    std::array<std::array<Line, 256 + 1>, 192 + 1> lines;
};

std::unique_ptr<Data> readFile(std::filesystem::path path) {
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

void testSlope(const Data &data, i32 testX, i32 testY, bool &mismatch) {
    // Helper function that prints the mismatch message on the first occurrence of a mismatch
    auto foundMismatch = [&] {
        if (!mismatch) {
            mismatch = true;
            std::cout << "found mismatch\n";
        }
    };

    // Create and configure the slopes
    //                   origins
    //              ltSlope  rtSlope
    // TEST_TOP       0,0    255,0
    // TEST_BOTTOM    0,191  255,191
    // TEST_LEFT      0,0      0,191
    // TEST_RIGHT   255,0    255,191

    Slope ltSlope; // left or top
    Slope rbSlope; // right or bottom
    i32 ltOriginX = (data.type != TEST_RIGHT) ? 0 : 255;
    i32 ltOriginY = (data.type != TEST_BOTTOM) ? 0 : 191;
    i32 rbOriginX = (data.type != TEST_LEFT) ? 255 : 0;
    i32 rbOriginY = (data.type != TEST_TOP) ? 191 : 0;
    ltSlope.Setup(ltOriginX, ltOriginY, testX, testY);
    rbSlope.Setup(rbOriginX, rbOriginY, testX, testY);

    i32 ltStartY = ltOriginY;
    i32 ltEndY = testY;
    i32 rbStartY = rbOriginY;
    i32 rbEndY = testY;

    auto adjustY = [&](i32 &startY, i32 &endY) {
        if (startY == endY) {
            // Y0 == Y1 is drawn as a single line
            endY++;
        } else if (startY > endY) {
            // Scan from top to bottom
            std::swap(startY, endY);
        }

        // Restrict to visible area
        if (endY > 191) {
            endY = 191;
        }
    };
    adjustY(ltStartY, ltEndY);
    adjustY(rbStartY, rbEndY);

    const i32 startY = std::min(ltStartY, rbStartY);
    const i32 endY = std::max(ltEndY, rbEndY);

    // Generate slopes and check the coverage values throughout the horizontal span
    for (i32 y = startY; y < endY; y++) {
        auto calcSlope = [&](const Slope &slope, std::string slopeName) {
            i32 startX = slope.XStart(y);
            i32 endX = slope.XEnd(y);
            if (slope.IsNegative()) {
                std::swap(startX, endX);
            }
            for (i32 x = startX; x <= endX; x++) {
                const i32 coverage = slope.AACoverage(x, y);
                const i32 fracCoverage = slope.FracAACoverage(x, y);
                const i32 aaFracBits = slope.IsXMajor() ? Slope::kAAFracBitsX : Slope::kAAFracBitsY;

                // Compare against data captured from hardware
                if (!data.lines[testY][testX].spans.contains(y)) {
                    if (coverage != 0) {
                        foundMismatch();

                        // clang-format off
                        std::cout << std::setw(3) << std::right << testX << "x" << std::setw(3) << std::left << testY;
                        std::cout << " @ " << std::setw(3) << std::right << x << "x" << std::setw(3) << std::left << y;
                        std::cout << "  " << slopeName << ": " << std::setw(2) << std::right << coverage << " != " << std::setw(2) << (u32)0;
                        std::cout << "  (" << std::setw(4) << fracCoverage << "  "
                            << std::setw(2) << std::right << (fracCoverage >> aaFracBits) << "."
                            << std::setw(2) << std::left << (fracCoverage & ((1 << aaFracBits) - 1)) << ")";
                            std::cout << "  AA step = " << slope.AAStep() << "  bias = " << slope.AABias();
                        std::cout << "\n";
                        // clang-format on
                    }
                } else {
                    auto &span = data.lines[testY][testX].spans.at(y);
                    if (coverage != span[x]) {
                        foundMismatch();

                        // clang-format off
                        std::cout << std::setw(3) << std::right << testX << "x" << std::setw(3) << std::left << testY;
                        std::cout << " @ " << std::setw(3) << std::right << x << "x" << std::setw(3) << std::left << y;
                        std::cout << "  " << slopeName << ": " << std::setw(2) << std::right << coverage << " != " << std::setw(2) << (u32)span[x];
                        std::cout << "  (" << std::setw(4) << fracCoverage << "  "
                            << std::setw(2) << std::right << (fracCoverage >> aaFracBits) << "."
                            << std::setw(2) << std::left << (fracCoverage & ((1 << aaFracBits) - 1)) << ")";
                            std::cout << "  AA step = " << slope.AAStep() << "  bias = " << slope.AABias();
                        std::cout << "\n";
                        // clang-format on
                    } else {
                        /*
                        // TODO: remove this branch
                        // clang-format off
                        std::cout << std::setw(3) << std::right << testX << "x" << std::setw(3) << std::left << testY;
                        std::cout << " @ " << std::setw(3) << std::right << x << "x" << std::setw(3) << std::left << y;
                        std::cout << "  " << slopeName << ": " << std::setw(2) << std::right << coverage << " == " <<
                        std::setw(2) << (u32)span[x]; std::cout << "  (" << std::setw(4) << fracCoverage << "  "
                            << std::setw(2) << std::right << (fracCoverage >> aaFracBits) << "."
                            << std::setw(2) << std::left << (fracCoverage & ((1 << aaFracBits) - 1)) << ")";
                            std::cout << "  AA step = " << slope.AAStep() << "  bias = " << slope.AABias();
                        std::cout << "\n";
                        // clang-format on
                        */
                    }
                }
            }
        };

        if (y >= ltStartY && y < ltEndY) {
            calcSlope(ltSlope, "LT");
        }
        // TODO: reenable this
        /*if (y >= rbStartY && y < rbEndY) {
            calcSlope(rbSlope, "RB");
        }*/
    }
}

void testSlopes(Data &data, i32 x0, i32 y0, const char *name) {
    std::cout << "Testing " << name << " slopes... ";

    bool mismatch = false;
    for (i32 y = data.minY; y <= data.maxY; y++) {
        for (i32 x = data.minX; x <= data.maxX; x++) {
            testSlope(data, x, y, mismatch);
        }
    }
    // testSlope(data, 3, 2, mismatch);
    // testSlope(data, 222, 3, mismatch);

    // Selected cases where LT breaks
    // testSlope(data, 54, 2, mismatch); // -1
    // testSlope(data, 65, 2, mismatch); // -1
    // testSlope(data, 227, 2, mismatch); // 3x -1
    // testSlope(data, 56, 5, mismatch); // 2x -1
    // testSlope(data, 15, 6, mismatch); // 2x -1
    // testSlope(data, 72, 73, mismatch); // +1, y-major
    // testSlope(data, 79, 73, mismatch); // +1
    // testSlope(data, 73, 108, mismatch); // -5, y-major
    // testSlope(data, 86, 108, mismatch); // 4x -many, y-major
    // testSlope(data, 110, 108, mismatch); // multiple errors
    // testSlope(data, 23, 150, mismatch); // -2
    // testSlope(data, 19, 152, mismatch); // multiple errors, y-major
    // testSlope(data, 116, 115, mismatch); // hardware produces odd output
    // testSlope(data, 176, 175, mismatch); // hardware produces odd output
    // testSlope(data, 185, 184, mismatch); // hardware produces odd output
    // testSlope(data, 186, 185, mismatch); // hardware produces odd output
    // testSlope(data, 188, 187, mismatch); // hardware produces odd output
    // testSlope(data, 189, 188, mismatch); // hardware produces odd output
    // testSlope(data, 191, 190, mismatch); // hardware produces odd output
    // testSlope(data, 192, 191, mismatch); // hardware produces odd output
    // testSlope(data, 236, 185, mismatch); // multiple significant errors
    // testSlope(data, 253, 126, mismatch);

    // [OK] Perfect diagonals -- should produce a coverage of 16 on every pixel
    /*for (u32 i = 1; i <= 192; i++) {
        testSlope(data, i, i, mismatch);
    }*/

    // [FAIL] X-major slopes "0" pixels tall (actually, 1 pixel tall)
    // Pixels 0x0 and 255x0 should have zero coverage
    /*for (u32 i = 1; i <= 256; i++) {
        testSlope(data, i, 0, mismatch);
    }*/

    // [OK] X-major slopes 1 pixel tall
    /*for (u32 i = 1; i <= 256; i++) {
        testSlope(data, i, 1, mismatch);
    }*/

    // [FAIL] X-major slopes 2 pixels tall
    /*for (u32 i = 2; i <= 256; i++) {
        testSlope(data, i, 2, mismatch);
    }*/

    // [FAIL] X-major slopes 5 pixels tall
    /*for (u32 i = 5; i <= 256; i++) {
        testSlope(data, i, 5, mismatch);
    }*/

    // [OK] Y-major slopes "0" pixels wide (actually, 1 pixel wide)
    /*for (u32 i = 1; i <= 192; i++) {
        testSlope(data, 0, i, mismatch);
    }*/

    // [FAIL] Y-major slopes 1 pixel wide
    /*for (u32 i = 1; i <= 192; i++) {
        testSlope(data, 1, i, mismatch);
    }*/

    // [FAIL] X-major slopes taller than 1 pixel
    /*testSlope(data, 9, 3, mismatch);
    testSlope(data, 10, 3, mismatch);
    testSlope(data, 100, 60, mismatch);
    testSlope(data, 155, 63, mismatch);*/

    // [FAIL] Y-major slopes wider than 1 pixel
    /*testSlope(data, 3, 9, mismatch);
    testSlope(data, 3, 10, mismatch);
    testSlope(data, 60, 100, mismatch);
    testSlope(data, 63, 155, mismatch);*/

    if (!mismatch) {
        std::cout << "OK!\n";
    }
}

void test(Data &data) {
    switch (data.type) {
    case 0: testSlopes(data, 0, 0, "top"); break;
    case 1: testSlopes(data, 256, 0, "bottom"); break;
    case 2: testSlopes(data, 0, 192, "left"); break;
    case 3: testSlopes(data, 256, 192, "right"); break;
    }
}

int main() {
    // convertScreenCap("data/screencap.bin", "data/screencap.tga");
    // uniqueColors("data/screencap.bin");

    // auto dataTL = readFile("data/TL.bin");
    // auto dataTR = readFile("data/TR.bin");
    // auto dataBL = readFile("data/BL.bin");
    // auto dataBR = readFile("data/BR.bin");

    auto dataT = readFile("C:/temp/T.bin");
    // auto dataB = readFile("C:/temp/B.bin");
    // auto dataL = readFile("C:/temp/L.bin");
    // auto dataR = readFile("C:/temp/R.bin");

    if (dataT)
        test(*dataT);
    // if (dataB)
    //    test(*dataB);
    // if (dataL)
    //    test(*dataL);
    // if (dataR)
    //    test(*dataR);

    // if (dataT) writeImages(*dataT, "C:/temp/T");
    // if (dataB) writeImages(*dataB, "C:/temp/B");
    // if (dataL) writeImages(*dataL, "C:/temp/L");
    // if (dataR) writeImages(*dataR, "C:/temp/R");

    return 0;
}
