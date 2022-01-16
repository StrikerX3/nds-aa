#include "tester.h"

#include "slope.h"

#include <iomanip>
#include <iostream>

void testSlope(const Data &data, i32 slopeWidth, i32 slopeHeight, bool &mismatch) {
    // Helper function that prints the mismatch message on the first occurrence of a mismatch
    auto foundMismatch = [&] {
        if (!mismatch) {
            mismatch = true;
            std::cout << "found mismatch\n";
        }
    };

    // Create and configure the slopes
    //                   origins            targets   (+ means w or h; - means 256-w or 192-h)
    //              ltSlope  rtSlope    ltSlope  rtSlope
    // TEST_TOP       0,0    256,0        +,+      -,+
    // TEST_BOTTOM    0,192  256,192      +,-      -,-
    // TEST_LEFT      0,0      0,192      +,+      +,-
    // TEST_RIGHT   256,0    256,192      -,+      -,-

    Slope ltSlope; // left or top
    Slope rbSlope; // right or bottom
    i32 ltOriginX = (data.type != TEST_RIGHT) ? 0 : 256;
    i32 ltOriginY = (data.type != TEST_BOTTOM) ? 0 : 192;
    i32 rbOriginX = (data.type != TEST_LEFT) ? 256 : 0;
    i32 rbOriginY = (data.type != TEST_TOP) ? 192 : 0;
    i32 ltTargetX = (data.type != TEST_RIGHT) ? slopeWidth : 256 - slopeWidth;
    i32 ltTargetY = (data.type != TEST_BOTTOM) ? slopeHeight : 192 - slopeHeight;
    i32 rbTargetX = (data.type != TEST_LEFT) ? 256 - slopeWidth : slopeWidth;
    i32 rbTargetY = (data.type != TEST_TOP) ? 192 - slopeHeight : slopeHeight;
    ltSlope.Setup(ltOriginX, ltOriginY, ltTargetX, ltTargetY);
    rbSlope.Setup(rbOriginX, rbOriginY, rbTargetX, rbTargetY);
    std::cout << ltSlope.Width() << "x" << ltSlope.Height() << " | " << rbSlope.Width() << "x" << rbSlope.Height()
              << "\n";

    i32 ltStartY = ltOriginY;
    i32 ltEndY = ltTargetY;
    i32 rbStartY = rbOriginY;
    i32 rbEndY = rbTargetY;

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

    // Generate slopes and check the coverage values
    for (i32 y = startY; y < endY; y++) {
        auto calcSlope = [&](const Slope &slope, std::string slopeName, i32 testX, i32 testY) {
            i32 startX = slope.XStart(y);
            i32 endX = slope.XEnd(y);
            i32 incX = slope.IsNegative() ? -1 : +1;
            if (data.type == TEST_LEFT && startX == 0) {
                // The Left test draws a vertical line on the left side of the screen with full coverage that has higher
                // priority over the tested slopes, producing a coverage value of 31 on every pixel at X=0. This line is
                // a left edge, which has precedence over right edges. We'll skip those pixels since the same cases are
                // tested on the Bottom and Top tests with no overridden pixels.
                startX++;
            }
            for (i32 x = startX; slope.IsNegative() ? x >= endX : x <= endX; x += incX) {
                const i32 coverage = slope.AACoverage(x, y);
                const i32 fracCoverage = slope.FracAACoverage(x, y);
                const i32 aaFracBits = Slope::kAAFracBits;

                // Compare against data captured from hardware
                auto &line = data.lines[testY][testX];
                u16 pixelIndex = (y << 8) | x;
                u8 pixel = line.pixels.contains(pixelIndex) ? line.pixels.at(pixelIndex) : 0;
                bool match = (coverage == pixel);
                if (!match) {
                    foundMismatch();
                }
                // TODO: print mismatches only
                std::cout << std::setw(3) << std::right << testX << "x" << std::setw(3) << std::left << testY  //
                          << " @ " << std::setw(3) << std::right << x << "x" << std::setw(3) << std::left << y //
                          << "  " << slopeName << ": " << std::setw(2) << std::right << coverage               //
                          << (match ? " == " : " != ") << std::setw(2) << (u32)pixel                           //
                          << "  (" << std::setw(4) << fracCoverage << "  "                                     //
                          << std::setw(2) << std::right << (fracCoverage >> aaFracBits) << "." << std::setw(2) //
                          << std::left << (fracCoverage & ((1 << aaFracBits) - 1)) << ")\n";
            }
        };

        if (y >= ltStartY && y < ltEndY) {
            calcSlope(ltSlope, "LT", ltTargetX, ltTargetY);
        }
        // TODO: reenable this
        /*if (y >= rbStartY && y < rbEndY) {
            calcSlope(rbSlope, "RB", rbTargetX, rbTargetY);
        }*/
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
    // testSlope(data, 256 - 15, 192 - 6, mismatch);
    // testSlope(data, 256 - 6, 192 - 15, mismatch);
    // testSlope(data, 2, 15, mismatch);
    // testSlope(data, 4, 30, mismatch);
    // testSlope(data, 24, 180, mismatch);
    // testSlope(data, 6, 15, mismatch);
    // testSlope(data, 6, 37, mismatch);
    // testSlope(data, 6, 6, mismatch);
    // testSlope(data, 84, 4, mismatch);
    testSlope(data, 186, 185, mismatch);
    // testSlope(data, 185, 186, mismatch);
    // testSlope(data, 54, 2, mismatch);

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