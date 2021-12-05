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
    i32 ltOriginX = (data.type != TEST_RIGHT) ? 0 : 256;
    i32 ltOriginY = (data.type != TEST_BOTTOM) ? 0 : 192;
    i32 rbOriginX = (data.type != TEST_LEFT) ? 256 : 0;
    i32 rbOriginY = (data.type != TEST_TOP) ? 192 : 0;
    ltSlope.Setup(ltOriginX, ltOriginY, testX, testY);
    rbSlope.Setup(rbOriginX, rbOriginY, testX, testY);
    std::cout << rbSlope.Width() << "x" << rbSlope.Height() << "\n";

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
        if (endY > 192) {
            endY = 192;
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
                    }
                }
            }
        };

        /*if (y >= ltStartY && y < ltEndY) {
            calcSlope(ltSlope, "LT");
        }*/
        // TODO: reenable this
        if (y >= rbStartY && y < rbEndY) {
            calcSlope(rbSlope, "RB");
        }
    }
}

void testSlopes(Data &data, i32 x0, i32 y0, const char *name) {
    std::cout << "Testing " << name << " slopes... ";

    bool mismatch = false;
    /*for (i32 y = data.minY; y <= data.maxY; y++) {
        for (i32 x = data.minX; x <= data.maxX; x++) {
            testSlope(data, x, y, mismatch);
        }
    }*/

    // All X-major slopes for TOP test, except Y=0
    /*for (i32 y = std::max<u8>(1, data.minY); y <= data.maxY; y++) {
        for (i32 x = y + 1; x <= data.maxX; x++) {
            testSlope(data, x, y, mismatch);
        }
    }*/
    // testSlope(data, 3, 2, mismatch);
    // testSlope(data, 222, 3, mismatch);
    testSlope(data, 256 - 15, 192 - 6, mismatch);

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

int mainX() {
    // convertScreenCap("data/screencap.bin", "data/screencap.tga");
    // uniqueColors("data/screencap.bin");

    // auto dataTL = readFile("data/TL.bin");
    // auto dataTR = readFile("data/TR.bin");
    // auto dataBL = readFile("data/BL.bin");
    // auto dataBR = readFile("data/BR.bin");

    // auto dataT = readFile("C:/temp/T.bin");
    // auto dataB = readFile("C:/temp/B.bin");
    // auto dataL = readFile("C:/temp/L.bin");
    auto dataR = readFile("C:/temp/R.bin");

    // if (dataT)
    //    test(*dataT);
    // if (dataB)
    //    test(*dataB);
    // if (dataL)
    //    test(*dataL);
    if (dataR)
        test(*dataR);

    // if (dataT) writeImages(*dataT, "C:/temp/T");
    // if (dataB) writeImages(*dataB, "C:/temp/B");
    // if (dataL) writeImages(*dataL, "C:/temp/L");
    // if (dataR) writeImages(*dataR, "C:/temp/R");

    return 0;
}

// --------------------------------------------------------------------------------

int main() {
    auto dataT = readFile("C:/temp/T.bin");
    auto dataB = readFile("C:/temp/B.bin");
    // auto dataL = readFile("C:/temp/L.bin");
    // auto dataR = readFile("C:/temp/R.bin");

    struct Output {
        Output(std::filesystem::path path)
            : stream{path, std::ios::binary} {}

        ~Output() {
            WriteTerminator();
        }

        std::ofstream stream;

        i32 lastWidth = -1;
        i32 lastHeight = -1;

        void Size(i32 width, i32 height) {
            if (width != lastWidth || height != lastHeight) {
                if (lastWidth != -1 && lastHeight != -1) {
                    WriteTerminator();
                }
                stream.write((const char *)&width, sizeof(u16));
                stream.write((const char *)&height, sizeof(u16));
                lastWidth = width;
                lastHeight = height;
            }
        }

    private:
        void WriteTerminator() {
            u32 terminator = 0xFFFFFFFF;
            stream.write((const char *)&terminator, sizeof(terminator));
        }
    };

    Output outLPX{"C:/temp/LPX.bin"};
    Output outLPY{"C:/temp/LPY.bin"};
    Output outLNX{"C:/temp/LNX.bin"};
    Output outLNY{"C:/temp/LNY.bin"};
    Output outRPX{"C:/temp/RPX.bin"};
    Output outRPY{"C:/temp/RPY.bin"};
    Output outRNX{"C:/temp/RNX.bin"};
    Output outRNY{"C:/temp/RNY.bin"};

    i32 minY = std::max(dataT->minY, dataB->minY);
    i32 maxY = std::min(dataT->maxY, dataB->maxY);
    i32 minX = std::max(dataT->minX, dataB->minX);
    i32 maxX = std::min(dataT->maxX, dataB->maxX);

    for (i32 y = minY; y <= maxY; y++) {
        for (i32 x = minX; x <= maxX; x++) {
            // Create and configure the slopes
            struct SlopeCoords {
                i32 startX, startY;
                i32 endX, endY;
            };

            SlopeCoords tltCoords{0, 0, x, y};
            SlopeCoords trbCoords{256, 0, x, y};
            SlopeCoords bltCoords{0, 192, x, y};
            SlopeCoords brbCoords{256, 192, x, y};

            auto adjustY = [&](SlopeCoords &coords) {
                if (coords.startY > coords.endY) {
                    // Scan from top to bottom
                    std::swap(coords.startX, coords.endX);
                    std::swap(coords.startY, coords.endY);
                }
            };
            adjustY(tltCoords);
            adjustY(trbCoords);
            adjustY(bltCoords);
            adjustY(brbCoords);

            Slope tltSlope{}; // top LT
            Slope trbSlope{}; // top RB
            Slope bltSlope{}; // bottom LT
            Slope brbSlope{}; // bottom RB
            tltSlope.Setup(tltCoords.startX, tltCoords.startY, tltCoords.endX, tltCoords.endY);
            trbSlope.Setup(trbCoords.startX, trbCoords.startY, trbCoords.endX, trbCoords.endY);
            bltSlope.Setup(bltCoords.startX, bltCoords.startY, bltCoords.endX, bltCoords.endY);
            brbSlope.Setup(brbCoords.startX, brbCoords.startY, brbCoords.endX, brbCoords.endY);

            auto calcSlope = [&](i32 xOffset, i32 startY, i32 yy, const Data &data, const Slope &slope, Output &out) {
                // Skip diagonals and perfect horizontals/verticals
                if (slope.Width() == 0 || slope.Height() == 0 || slope.Width() == slope.Height()) {
                    return;
                }

                // Determine horizontal span
                i32 startX = slope.XStart(yy);
                i32 endX = slope.XEnd(yy);
                if (slope.IsNegative()) {
                    std::swap(startX, endX);
                }

                // Write slope values from hardware capture using the generated slope
                // Data format:
                //   [u16] width
                //   [u16] height
                //   repeated:
                //     [u16] x
                //     [u8] y
                //     [u8] expected output
                //     (if all of these are FFs, end of list)
                const i32 w = slope.Width();
                const i32 h = slope.Height();
                out.Size(w, h);
                auto &spans = data.lines[y][x].spans;
                for (i32 xx = startX; xx <= endX; xx++) {
                    if (spans.contains(yy)) {
                        auto &span = spans.at(yy);
                        // Contents:
                        // - input: width,height; x,y coordinates
                        // - output: expected coverage value at x,y (including zeros)
                        i32 oxx = xx - xOffset;
                        i32 oyy = yy - startY;
                        out.stream.write((const char *)&oxx, sizeof(u16));
                        out.stream.write((const char *)&oyy, sizeof(u8));
                        out.stream.write((const char *)&span[xx], sizeof(u8));
                    }
                }
            };

            // Generate slopes and write coverage values to the corresponding data set
            for (i32 yy = 0; yy < y; yy++) {
                calcSlope(std::min(tltCoords.startX, tltCoords.endX), 0, yy, *dataT, tltSlope,
                          tltSlope.IsXMajor() ? outLPX : outLPY);
                calcSlope(std::min(trbCoords.startX, trbCoords.endX), 0, yy, *dataT, trbSlope,
                          trbSlope.IsXMajor() ? outRNX : outRNY);
            }
            for (i32 yy = y; yy < 192; yy++) {
                calcSlope(std::min(bltCoords.startX, bltCoords.endX), y, yy, *dataB, bltSlope,
                          bltSlope.IsXMajor() ? outLNX : outLNY);
                calcSlope(std::min(brbCoords.startX, brbCoords.endX), y, yy, *dataB, brbSlope,
                          brbSlope.IsXMajor() ? outRPX : outRPY);
            }
        }
    }

    return EXIT_SUCCESS;
}

// --------------------------------------------------------------------------------

#include <map>
#include <random>
#include <vector>

enum class Operator {
    FracXStart,
    FracXEnd,
    XStart,
    XEnd,
    XWidth,
    ExpandAABits,
    MulWidth,
    MulHeight,
    DivWidth,
    DivHeight,
    Div2,
    MulHeightDivWidthAA,
    PushX,
    PushY,
    PushWidth,
    PushHeight,

    Add,
    Subtract,
    Multiply,
    Divide,
    Modulo,
    Negate,
    LeftShift,
    ArithmeticRightShift,
    LogicRightShift,
    And,
    Or,
    Xor,
    Not,
    Dup,
    Swap,
};

struct Variables {
    i32 x, y;
    i32 width, height;
};

struct Context {
    Variables vars;

private:
    std::vector<i32> stack;

    friend struct Operation;
    friend struct Evaluator;
};

struct Operation {
    enum class Type { Operator, Constant };

    Type type;
    union {
        Operator op;
        i32 constVal;
        i32 varId;
    };

    std::string Str() {
        switch (type) {
        case Type::Operator:
            switch (op) {
            case Operator::FracXStart: return "push_frac_x_start";
            case Operator::FracXEnd: return "push_frac_x_end";
            case Operator::XStart: return "push_x_start";
            case Operator::XEnd: return "push_x_end";
            case Operator::XWidth: return "push_x_width";
            case Operator::ExpandAABits: return "expand_aa_bits";
            case Operator::MulWidth: return "mul_width";
            case Operator::MulHeight: return "mul_height";
            case Operator::DivWidth: return "div_width";
            case Operator::DivHeight: return "div_height";
            case Operator::Div2: return "div_2";
            case Operator::MulHeightDivWidthAA: return "mul_height_div_width_aa";
            case Operator::PushX: return "push_x";
            case Operator::PushY: return "push_y";
            case Operator::PushWidth: return "push_width";
            case Operator::PushHeight: return "push_height";

            case Operator::Add: return "add";
            case Operator::Subtract: return "sub";
            case Operator::Multiply: return "mul";
            case Operator::Divide: return "div";
            case Operator::Modulo: return "mod";
            case Operator::Negate: return "neg";
            case Operator::LeftShift: return "shl";
            case Operator::ArithmeticRightShift: return "sar";
            case Operator::LogicRightShift: return "shr";
            case Operator::And: return "and";
            case Operator::Or: return "or";
            case Operator::Xor: return "xor";
            case Operator::Not: return "not";
            case Operator::Dup: return "dup";
            case Operator::Swap: return "swap";
            }
            return "(invalid op)";
        case Type::Constant:
            return std::string("push_const ") + std::to_string(constVal);
            // return std::format("push_const {}", constVal);
        default: return "(invalid type)";
        }
    }

    [[gnu::flatten]] bool Execute(Context &ctx) {
        switch (type) {
        case Type::Operator: return ExecuteOperator(ctx);
        case Type::Constant: return ExecuteConstant(ctx);
        }
        return false;
    }

private:
    bool ExecuteOperator(Context &ctx) {
        auto &stack = ctx.stack;
        auto binaryFunc = [&](auto &&func) -> bool {
            if (stack.size() < 2) {
                return false;
            }
            const i32 y = stack.back();
            stack.pop_back();
            const i32 x = stack.back();
            stack.pop_back();
            const i32 result = func(x, y);
            stack.push_back(result);
            return true;
        };

        auto unaryFunc = [&](auto &&func) -> bool {
            if (stack.size() < 1) {
                return false;
            }
            const i32 x = stack.back();
            stack.pop_back();
            const i32 result = func(x);
            stack.push_back(result);
            return true;
        };

        switch (op) {
        case Operator::FracXStart: {
            i32 divResult = (Slope::kOne / ctx.vars.height);
            i32 dx = ctx.vars.y * divResult * ctx.vars.width;
            stack.push_back(Slope::kBias + dx);
            return true;
        }
        case Operator::FracXEnd: {
            i32 divResult = (Slope::kOne / ctx.vars.height);
            i32 dx = ctx.vars.y * divResult * ctx.vars.width;
            i32 fracStart = Slope::kBias + dx;
            return (fracStart & Slope::kMask) + dx - Slope::kOne;
        }

        case Operator::XStart: {
            i32 divResult = (Slope::kOne / ctx.vars.height);
            i32 dx = ctx.vars.y * divResult * ctx.vars.width;
            stack.push_back((Slope::kBias + dx) >> Slope::kFracBits);
            return true;
        }
        case Operator::XEnd: {
            i32 divResult = (Slope::kOne / ctx.vars.height);
            i32 dx = ctx.vars.y * divResult * ctx.vars.width;
            i32 fracStart = Slope::kBias + dx;
            return ((fracStart & Slope::kMask) + dx - Slope::kOne) >> Slope::kFracBits;
        }

        case Operator::XWidth: {
            i32 divResult = (Slope::kOne / ctx.vars.height);
            i32 dx = ctx.vars.y * divResult * ctx.vars.width;
            i32 fracStart = Slope::kBias + dx;
            i32 xStart = fracStart >> Slope::kFracBits;
            i32 xEnd = ((fracStart & Slope::kMask) + dx - Slope::kOne) >> Slope::kFracBits;
            return xEnd - xStart + 1;
        }
        case Operator::ExpandAABits: return unaryFunc([](i32 x) { return (x * 32) << Slope::kAAFracBitsX; });
        case Operator::MulWidth: return unaryFunc([&](i32 x) { return x * ctx.vars.width; });
        case Operator::MulHeight: return unaryFunc([&](i32 x) { return x * ctx.vars.height; });
        case Operator::DivWidth: return unaryFunc([&](i32 x) { return x / ctx.vars.width; });
        case Operator::DivHeight: return unaryFunc([&](i32 x) { return x / ctx.vars.height; });
        case Operator::Div2: return unaryFunc([&](i32 x) { return x >> 1; });
        case Operator::MulHeightDivWidthAA:
            return unaryFunc(
                [&](i32 x) { return ((x * ctx.vars.height * 32) << Slope::kAAFracBitsX) / ctx.vars.width; });
        case Operator::PushX: ctx.stack.push_back(ctx.vars.x); return true;
        case Operator::PushY: ctx.stack.push_back(ctx.vars.y); return true;
        case Operator::PushWidth: ctx.stack.push_back(ctx.vars.width); return true;
        case Operator::PushHeight: ctx.stack.push_back(ctx.vars.height); return true;

        case Operator::Add: return binaryFunc([](i32 x, i32 y) { return x + y; });
        case Operator::Subtract: return binaryFunc([](i32 x, i32 y) { return x - y; });
        case Operator::Multiply: return binaryFunc([](i32 x, i32 y) { return x * y; });
        case Operator::Divide:
            return binaryFunc([](i32 x, i32 y) { return y == 0 || (x == 0x80000000 && y == -1) ? INT32_MAX : x / y; });
        case Operator::Modulo:
            return binaryFunc([](i32 x, i32 y) { return y == 0 || (x == 0x80000000 && y == -1) ? 0 : x % y; });
        case Operator::Negate: return unaryFunc([](i32 x) { return -x; });
        case Operator::LeftShift: return binaryFunc([](i32 x, i32 y) { return x << y; });
        case Operator::ArithmeticRightShift: return binaryFunc([](i32 x, i32 y) { return x >> y; });
        case Operator::LogicRightShift: return binaryFunc([](i32 x, i32 y) { return (u32)x >> (u32)y; });
        case Operator::And: return binaryFunc([](i32 x, i32 y) { return x & y; });
        case Operator::Or: return binaryFunc([](i32 x, i32 y) { return x | y; });
        case Operator::Xor: return binaryFunc([](i32 x, i32 y) { return x ^ y; });
        case Operator::Not: return unaryFunc([](i32 x) { return ~x; });
        case Operator::Dup:
            if (stack.empty()) {
                return false;
            } else {
                stack.push_back(stack.back());
                return true;
            }
        case Operator::Swap:
            if (stack.size() < 2) {
                return false;
            } else {
                std::swap(stack[stack.size() - 1], stack[stack.size() - 2]);
                return true;
            }
        }
        return false;
    }

    bool ExecuteConstant(Context &ctx) {
        ctx.stack.push_back(constVal);
        return true;
    }
};

struct Evaluator {
    Context ctx;
    std::vector<Operation> ops;

    void DebugPrint(std::ostream &os) {
        os << "Variables:\n";
        os << "  x=" << ctx.vars.x << "\n";
        os << "  y=" << ctx.vars.y << "\n";
        os << "  w=" << ctx.vars.width << "\n";
        os << "  h=" << ctx.vars.height << "\n";

        os << "Operations:\n";
        for (auto &op : ops) {
            os << "  " << op.Str() << "\n";
        }
    }

    bool Eval(i32 &result) {
        ctx.stack.clear();
        for (auto &op : ops) {
            if (!op.Execute(ctx)) {
                return false;
            }
        }
        if (ctx.stack.size() != 1) {
            return false;
        }
        const i32 divResult = (Slope::kOne / ctx.vars.height);
        const i32 dx = ctx.vars.y * divResult * ctx.vars.width;
        const i32 fracStart = Slope::kBias + dx;
        const i32 startX = fracStart >> Slope::kFracBits;
        const i32 endX = ((fracStart & Slope::kMask) + dx - Slope::kOne) >> Slope::kFracBits;
        const i32 deltaX = endX - startX + 1;
        const i32 baseCoverage = ctx.stack.back();
        const i32 fullCoverage = ((deltaX * ctx.vars.height * Slope::kAARange) << Slope::kAAFracBitsX) / ctx.vars.width;
        const i32 coverageStep = fullCoverage / deltaX;
        const i32 coverageBias = coverageStep / 2;
        const i32 offset = ctx.vars.x - startX;
        const i32 fracCoverage = baseCoverage + offset * coverageStep;
        const i32 finalCoverage = (fracCoverage + coverageBias) % Slope::kAABaseX;
        result = finalCoverage >> Slope::kAAFracBitsX;

        // result = ctx.stack.back();

        return true;
    }
};

struct DataPoint {
    i32 x, y;
    i32 width, height;
    i32 expectedOutput;

    void ApplyVars(Variables &vars) {
        vars.x = x;
        vars.y = y;
        vars.width = width;
        vars.height = height;
    }
};

int main3() {
    std::vector<DataPoint> dataPoints;
    // dataPoints.push_back(DataPoint{.x = 36, .y = 1, .width = 54, .height = 2, .expectedOutput = 11});
    // dataPoints.push_back(DataPoint{.x = 2, .y = 1, .width = 15, .height = 6, .expectedOutput = 0});
    // dataPoints.push_back(DataPoint{.x = 7, .y = 3, .width = 15, .height = 6, .expectedOutput = 0});
    // dataPoints.push_back(DataPoint{.x = 9, .y = 0, .width = 54, .height = 2, .expectedOutput = 10});
    // dataPoints.push_back(DataPoint{.x = 12, .y = 5, .width = 15, .height = 6, .expectedOutput = 0});
    // dataPoints.push_back(DataPoint{.x = 44, .y = 3, .width = 56, .height = 5, .expectedOutput = 31});
    // dataPoints.push_back(DataPoint{.x = 45, .y = 4, .width = 56, .height = 5, .expectedOutput = 2});
    dataPoints.push_back(DataPoint{.x = 106, .y = 106, .width = 186, .height = 185, .expectedOutput = 2});
    dataPoints.push_back(DataPoint{.x = 107, .y = 106, .width = 186, .height = 185, .expectedOutput = 31});

    dataPoints.push_back(DataPoint{.x = 0, .y = 0, .width = 15, .height = 6, .expectedOutput = 6});
    dataPoints.push_back(DataPoint{.x = 1, .y = 0, .width = 15, .height = 6, .expectedOutput = 19});
    dataPoints.push_back(DataPoint{.x = 35, .y = 1, .width = 54, .height = 2, .expectedOutput = 9});
    dataPoints.push_back(DataPoint{.x = 3, .y = 1, .width = 15, .height = 6, .expectedOutput = 12});
    dataPoints.push_back(DataPoint{.x = 105, .y = 105, .width = 186, .height = 185, .expectedOutput = 29});
    dataPoints.push_back(DataPoint{.x = 37, .y = 1, .width = 54, .height = 2, .expectedOutput = 12});
    dataPoints.push_back(DataPoint{.x = 4, .y = 1, .width = 15, .height = 6, .expectedOutput = 25});
    dataPoints.push_back(DataPoint{.x = 43, .y = 3, .width = 56, .height = 5, .expectedOutput = 28});
    dataPoints.push_back(DataPoint{.x = 6, .y = 2, .width = 15, .height = 6, .expectedOutput = 19});
    dataPoints.push_back(DataPoint{.x = 10, .y = 4, .width = 15, .height = 6, .expectedOutput = 6});
    dataPoints.push_back(DataPoint{.x = 108, .y = 107, .width = 186, .height = 185, .expectedOutput = 29});
    dataPoints.push_back(DataPoint{.x = 8, .y = 0, .width = 54, .height = 2, .expectedOutput = 9});
    dataPoints.push_back(DataPoint{.x = 46, .y = 4, .width = 56, .height = 5, .expectedOutput = 4});
    dataPoints.push_back(DataPoint{.x = 92, .y = 92, .width = 186, .height = 185, .expectedOutput = 0});
    dataPoints.push_back(DataPoint{.x = 93, .y = 93, .width = 186, .height = 185, .expectedOutput = 31});

    dataPoints.push_back(DataPoint{.x = 5, .y = 2, .width = 15, .height = 6, .expectedOutput = 6});
    dataPoints.push_back(DataPoint{.x = 8, .y = 3, .width = 15, .height = 6, .expectedOutput = 12});
    dataPoints.push_back(DataPoint{.x = 9, .y = 3, .width = 15, .height = 6, .expectedOutput = 25});
    dataPoints.push_back(DataPoint{.x = 11, .y = 4, .width = 15, .height = 6, .expectedOutput = 19});
    dataPoints.push_back(DataPoint{.x = 13, .y = 5, .width = 15, .height = 6, .expectedOutput = 12});
    dataPoints.push_back(DataPoint{.x = 14, .y = 5, .width = 15, .height = 6, .expectedOutput = 25});
    dataPoints.push_back(DataPoint{.x = 10, .y = 0, .width = 54, .height = 2, .expectedOutput = 12});

    Evaluator eval;

    constexpr i32 kConstants[] = {
        // 1,
        // Slope::kAARange, Slope::kAAFracBitsX, Slope::kAABaseX,
        // Slope::kFracBits, Slope::kFracBits >> 1,
        // Slope::kOne, Slope::kBias,
    };
    std::vector<Operation> templateOps;
    {
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::PushX});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::PushY});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::PushWidth});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::PushHeight});
        for (i32 c : kConstants) {
            templateOps.push_back(Operation{.type = Operation::Type::Constant, .constVal = c});
        }
        // templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::FracXStart});
        // templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::FracXEnd});

        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::XStart});
        // templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::XEnd});
        // templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::XWidth});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::ExpandAABits});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::MulWidth});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::MulHeight});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::DivWidth});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::DivHeight});
        // templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Div2});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::MulHeightDivWidthAA});

        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Add});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Subtract});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Multiply});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Divide});
        // templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Modulo});
        // templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Negate});
        // templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::LeftShift});
        // templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::ArithmeticRightShift});
        // templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::LogicRightShift});
        // templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::And});
        // templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Or});
        // templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Xor});
        // templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Not});
        // templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Dup});
        // templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Swap});
    }

    std::vector<size_t> templateIndices;

    auto nextTemplate = [&] {
        if (templateIndices.empty()) {
            templateIndices.push_back(0);
            eval.ops.push_back(templateOps[0]);
        } else {
            size_t index = 0;
            for (;;) {
                templateIndices[index]++;
                if (templateIndices[index] == templateOps.size()) {
                    templateIndices[index] = 0;
                    index++;
                    if (index == templateIndices.size()) {
                        templateIndices.push_back(0);
                        eval.ops.push_back(templateOps[0]);
                        std::cout << templateIndices.size() << "\n";
                        break;
                    }
                } else {
                    break;
                }
            }
            for (size_t i = 0; i <= index; i++) {
                eval.ops[i] = templateOps[templateIndices[i]];
            }
        }
    };

    /*std::default_random_engine gen;
    std::uniform_int_distribution<int> distSize(8, 12);
    std::uniform_int_distribution<int> distOp(0, templateOps.size() - 1);
    auto randomTemplate = [&] {
        eval.ops.resize(distSize(gen));
        for (size_t i = 0; i < eval.ops.size(); i++) {
            eval.ops[i] = templateOps[distOp(gen)];
        }
    };*/

    auto t1 = std::chrono::steady_clock::now();
    std::vector<Operation> resultOps;
    for (;;) {
        nextTemplate();
        // randomTemplate();

        bool valid = true;
        for (auto &dataPoint : dataPoints) {
            dataPoint.ApplyVars(eval.ctx.vars);
            if (i32 result; eval.Eval(result)) {
                // result = (result >> Slope::kAAFracBitsX) % Slope::kAARange;
                // result = result % Slope::kAARange;
                if (result != dataPoint.expectedOutput) {
                    valid = false;
                    break;
                }
            } else {
                valid = false;
                break;
            }
        }
        if (valid) {
            std::cout << "Found!\n";
            resultOps = eval.ops;
            break;
        }
    }
    auto t2 = std::chrono::steady_clock::now();
    auto dt = t2 - t1;
    std::cout << "Time: " << std::chrono::duration_cast<std::chrono::milliseconds>(dt) << "\n";

    if (!resultOps.empty()) {
        std::cout << "Operations:\n";
        for (auto &op : resultOps) {
            std::cout << "  " << op.Str() << "\n";
        }
        eval.ops = resultOps;
        std::cout << "Actual results:\n";
        for (auto &dataPoint : dataPoints) {
            dataPoint.ApplyVars(eval.ctx.vars);
            if (i32 result; eval.Eval(result)) {
                // result = (result >> Slope::kAAFracBitsX) % Slope::kAARange;
                // result = result % Slope::kAARange;
                std::cout << dataPoint.width << "x" << dataPoint.height << " @ " << dataPoint.x << "x" << dataPoint.y
                          << "  " << result << (result == dataPoint.expectedOutput ? " == " : " != ")
                          << dataPoint.expectedOutput << "\n";
            }
        }
    } else {
        std::cout << "Could not find sequence of operations that matches the given data points\n";
    }

    return EXIT_SUCCESS;
}
