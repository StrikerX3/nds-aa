#include "tester.h"

#include "slope.h"

#include <iomanip>
#include <iostream>

struct TestResult {
    bool mismatch = false;
    u64 numMatches = 0;
    u64 testedPixels = 0;
    u64 overshoot = 0;
    u64 undershoot = 0;
};

void testSlope(const Data &data, i32 slopeWidth, i32 slopeHeight, TestResult &result) {
    // Helper function that prints the mismatch message on the first occurrence of a mismatch
    auto foundMismatch = [&] {
        if (!result.mismatch) {
            result.mismatch = true;
            // std::cout << "found mismatch\n";
        }
    };

    // Create and configure the slopes
    //              origins            targets   (+ means w or h; - means 256-w or 192-h)
    //         ltSlope  rtSlope    ltSlope  rtSlope
    // TOP       0,0    256,0        +,+      -,+
    // BOTTOM    0,192  256,192      +,-      -,-
    // LEFT      0,0      0,192      +,+      +,-
    // RIGHT   256,0    256,192      -,+      -,-

    // Edge side per test:
    //           LT    RB
    // TOP      left  right
    // BOTTOM   left  right
    // LEFT     right right
    // RIGHT    left  left

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
    ltSlope.Setup(ltOriginX, ltOriginY, ltTargetX, ltTargetY, data.type != TEST_LEFT);
    rbSlope.Setup(rbOriginX, rbOriginY, rbTargetX, rbTargetY, data.type == TEST_RIGHT);
    /*std::cout << ltSlope.Width() << "x" << ltSlope.Height() << " | " << rbSlope.Width() << "x" << rbSlope.Height()
              << "\n";*/

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

    // Dump gradients from the data set
    /*auto dumpGradient = [&](Slope &slope, i32 targetX, i32 targetY, i32 startY, i32 endY) {
        // Avoid division by zero.
        // Also, we're not interested in perfectly vertical or horizontal edges; those are solved already.
        if (slope.Width() == 0 || slope.Height() == 0) {
            return;
        }

        const i32 gradFlip = (slope.IsLeftEdge() == (slope.IsNegative() == slope.IsXMajor())) ? 31 : 0;
        const i32 aaStep = slope.IsXMajor() //
                               ? slope.Height() * 1024 / slope.Width()
                               : slope.Width() * 1024 / slope.Height();

        std::cout << std::setw(3) << std::right << slope.Width() << 'x' << std::setw(3) << std::left << slope.Height();
        std::cout << "  aa step=" << aaStep;
        std::cout << "   "                            //
                  << (slope.IsLeftEdge() ? 'L' : 'R') //
                  << (slope.IsPositive() ? 'P' : 'N') //
                  << (slope.IsXMajor() ? 'X' : 'Y')   //
                  << '\n';

        if (slope.IsXMajor()) {
            for (i32 y = startY; y < endY; y++) {
                i32 startX = slope.XStart(y);
                i32 endX = slope.XEnd(y);
                bool flipped = (startX > endX);
                if (flipped) {
                    std::swap(startX, endX);
                }

                // All tests draw a triangle with one edge covering the entire span of the screen border given by the
                // test name. Due to polygon drawing rules and edge precedences, in some cases these pixels will
                // override the tested slopes with pixels of full coverage, producing false negatives if checked
                // blindly. The following conditions skip such pixels.
                if (data.type == TEST_LEFT && startX == 0) {
                    // The Left test draws a vertical line on the left side of the screen which is considered a left
                    // edge in all cases, and thus overrides all pixels at X=0.
                    startX++;
                }

                // Determine the valid bias range
                i32 biasLowerBound = 0;
                i32 biasUpperBound = 0;
                i32 baseCoverage = 0;
                auto &line = data.lines[targetY][targetX];

                auto aaCovLower = [&] {
                    return std::min((baseCoverage + biasLowerBound) >> 5, 31) ^ gradFlip ^ (flipped * 31);
                };
                auto aaCovUpper = [&] {
                    return std::min((baseCoverage + biasUpperBound) >> 5, 31) ^ gradFlip ^ (flipped * 31);
                };

                // Do a forward scan to find the lower bound
                for (i32 x = startX; x <= endX; x++) {
                    // Ignore the leftmost pixel of 256-wide right slopes because they make it impossible to find a
                    // valid bias for a gradient
                    if (slope.Width() == 256 && slope.IsRightEdge() && x == 0) {
                        continue;
                    }
                    while (biasLowerBound < 1024 && aaCovLower() != line.Pixel(x, y)) {
                        biasLowerBound++;
                    }
                    baseCoverage += aaStep;
                }
                baseCoverage -= aaStep;

                // Extend upper bound as far as the last value in the gradient allows
                biasUpperBound = biasLowerBound;
                while (biasUpperBound < 1024 && aaCovUpper() == line.Pixel(endX, y)) {
                    biasUpperBound++;
                }

                // Do a backward scan to find the upper bound
                for (i32 x = endX; x >= startX; x--) {
                    // Ignore the leftmost pixel of 256-wide right slopes because they make it impossible to find a
                    // valid bias for a gradient
                    if (slope.Width() == 256 && slope.IsRightEdge() && x == 0) {
                        continue;
                    }
                    while (biasUpperBound > 0 && aaCovUpper() != line.Pixel(x, y)) {
                        biasUpperBound--;
                    }
                    baseCoverage -= aaStep;
                }

                // Display gradient
                std::cout << "    y=" << std::setw(3) << std::left << y << "  ";
                std::cout << std::setw(3) << std::right << startX << ".." << std::setw(3) << std::left << endX
                          << " -> ";
                if (biasLowerBound >= 1024) {
                    std::cout << "(unexpected gradient)";
                } else {
                    std::cout << std::setw(4) << std::right << biasLowerBound;
                    if (biasLowerBound != biasUpperBound) {
                        std::cout << ".." << std::setw(4) << std::left << biasUpperBound;
                    } else {
                        std::cout << "      ";
                    }
                }
                std::cout << "  ";
                for (i32 x = startX; x <= endX; x++) {
                    std::cout << " " << std::setw(2) << std::right << (u32)line.Pixel(x, y);
                }
                std::cout << '\n';
            }
        } else { // Y-major or diagonal
            i32 lastX = -1;
            i32 topY = -1;
            for (i32 y = startY; y < endY; y++) {
                i32 x = slope.XStart(y);

                // All tests draw a triangle with one edge covering the entire span of the screen border given by the
                // test name. Due to polygon drawing rules and edge precedences, in some cases these pixels will
                // override the tested slopes with pixels of full coverage, producing false negatives if checked
                // blindly. The following conditions skip such pixels.
                if (data.type == TEST_LEFT && x == 0) {
                    // The Left test draws a vertical line on the left side of the screen which is considered a left
                    // edge in all cases, and thus overrides all pixels at X=0.
                    continue;
                }

                // We want to process individual vertical segments of the slope in one go.
                // We know we've reached the end of a segment when the X coordinate changes or we're at the last Y.
                if (x == lastX && y < endY - 1) {
                    continue;
                }

                if (lastX == -1) {
                    // First pixel of the slope; initialize parameters
                    lastX = x;
                    topY = y;
                    continue;
                }

                // X changed or we reached the bottom of the slope, so process the current segment
                const i32 xx = lastX;
                const i32 btmY = (y == endY - 1) ? y : std::max(topY, y - 1);

                // Determine the valid bias range
                i32 biasLowerBound = 0;
                i32 biasUpperBound = 0;
                i32 baseCoverage = 0;
                auto &line = data.lines[targetY][targetX];

                auto aaCovLower = [&] { return std::min((baseCoverage + biasLowerBound) >> 5, 31) ^ gradFlip; };
                auto aaCovUpper = [&] { return std::min((baseCoverage + biasUpperBound) >> 5, 31) ^ gradFlip; };

                // Do a forward scan to find the lower bound
                for (i32 yy = topY; yy <= btmY; yy++) {
                    // Ignore the bottommost pixel of every slice since they have a fixed value that confuses the
                    // gradient finder algorithm
                    if (yy == btmY && btmY < endY - 1) {
                        continue;
                    }
                    while (biasLowerBound < 1024 && aaCovLower() != line.Pixel(xx, yy)) {
                        biasLowerBound++;
                    }
                    baseCoverage += aaStep;
                }
                baseCoverage -= aaStep;

                // Extend upper bound as far as the last value in the gradient allows
                biasUpperBound = biasLowerBound;
                i32 checkY = btmY;
                if (btmY < endY - 1) {
                    // Ignore the bottommost pixel of every slice since they have a fixed value that confuses the
                    // gradient finder algorithm
                    checkY--;
                }
                if (checkY >= topY) {
                    while (biasUpperBound < 1024 && aaCovUpper() == line.Pixel(xx, checkY)) {
                        biasUpperBound++;
                    }
                }

                // Do a backward scan to find the upper bound
                for (i32 yy = btmY; yy >= topY; yy--) {
                    // Ignore the bottommost pixel of every slice since they have a fixed value that confuses the
                    // gradient finder algorithm
                    if (yy == btmY && btmY < endY - 1) {
                        continue;
                    }
                    while (biasUpperBound > 0 && aaCovUpper() != line.Pixel(xx, yy)) {
                        biasUpperBound--;
                    }
                    baseCoverage -= aaStep;
                }

                // Display gradient
                std::cout << "    x=" << std::setw(3) << std::left << xx << "  ";
                std::cout << std::setw(3) << std::right << topY << ".." << std::setw(3) << std::left << btmY << " -> ";
                if (biasLowerBound >= 1024) {
                    std::cout << "(unexpected gradient)";
                } else {
                    std::cout << std::setw(4) << std::right << biasLowerBound;
                    if (biasLowerBound != biasUpperBound) {
                        std::cout << ".." << std::setw(4) << std::left << biasUpperBound;
                    } else {
                        std::cout << "      ";
                    }
                }
                std::cout << "  ";
                for (i32 yy = topY; yy <= btmY; yy++) {
                    std::cout << " " << std::setw(2) << std::right << (u32)line.Pixel(xx, yy);
                }
                std::cout << '\n';

                // Update segment parameters for the next iteration
                lastX = x;
                topY = y;
            }
        }
    };
    dumpGradient(ltSlope, ltTargetX, ltTargetY, ltStartY, ltEndY);
    dumpGradient(rbSlope, rbTargetX, rbTargetY, rbStartY, rbEndY);*/

    // Generate X-major gradients using the new bias method and compare against the data set
    /*auto calcGradient = [&](Slope &slope, i32 targetX, i32 targetY, i32 startY, i32 endY) {
        if (slope.IsXMajor() && slope.Width() > 0) {
            const i32 gradFlip = (slope.IsLeftEdge() == (slope.IsNegative() == slope.IsXMajor())) ? 31 : 0;
            const i32 aaStep = slope.Height() * 1024 / slope.Width();
            std::cout << std::setw(3) << std::right << slope.Width() << 'x' << std::setw(3) << std::left
                      << slope.Height();
            std::cout << "  aa step=" << aaStep;
            std::cout << "   "                            //
                      << (slope.IsLeftEdge() ? 'L' : 'R') //
                      << (slope.IsPositive() ? 'P' : 'N') //
                      << (slope.IsXMajor() ? 'X' : 'Y')   //
                      << '\n';
            for (i32 y = startY; y < endY; y++) {
                i32 left = slope.IsNegative() ? slope.X0() - slope.Width() : slope.X0();
                i32 startX = slope.XStart(y);
                i32 endX = slope.XEnd(y);
                const i32 incX = slope.IsNegative() ? -1 : +1;

                // All tests draw a triangle with one edge covering the entire span of the screen border given by the
                // test name. Due to polygon drawing rules and edge precedences, in some cases these pixels will
                // override the tested slopes with pixels of full coverage, producing false negatives if checked
                // blindly. The following conditions skip such pixels.
                if (data.type == TEST_LEFT && startX == 0) {
                    // The Left test draws a vertical line on the left side of the screen which is considered a left
                    // edge in all cases, and thus overrides all pixels at X=0.
                    startX++;
                }
                if ((data.type == TEST_TOP || data.type == TEST_BOTTOM) && slope.Height() == 0) {
                    // The Top and bottom tests draw a horizontal line at the top or bottom of the screen. Only the
                    // leftmost pixel of the left edge is valid.
                    if (slope.IsNegative() == slope.IsLeftEdge()) {
                        startX = endX;
                    } else {
                        endX = startX;
                    }
                }

                // Determine the valid bias range
                i32 biasLowerBound = 0;
                i32 biasUpperBound = 0;
                i32 baseCoverage = 0;
                auto &line = data.lines[targetY][targetX];

                auto aaCovLower = [&] { return std::min((baseCoverage + biasLowerBound) >> 5, 31) ^ gradFlip; };
                auto aaCovUpper = [&] { return std::min((baseCoverage + biasUpperBound) >> 5, 31) ^ gradFlip; };

                // Do a forward scan to find the lower bound
                for (i32 x = startX; slope.IsNegative() ? x >= endX : x <= endX; x += incX) {
                    while (biasLowerBound < 1024 && aaCovLower() != line.Pixel(x, y)) {
                        biasLowerBound++;
                    }
                    baseCoverage += aaStep;
                }
                baseCoverage -= aaStep;

                // Extend upper bound as far as the last value in the gradient allows
                biasUpperBound = biasLowerBound;
                while (biasUpperBound < 1024 && aaCovUpper() == line.Pixel(endX, y)) {
                    biasUpperBound++;
                }

                // Do a backward scan to find the upper bound
                for (i32 x = endX; slope.IsNegative() ? x <= startX : x >= startX; x -= incX) {
                    while (biasUpperBound > 0 && aaCovUpper() != line.Pixel(x, y)) {
                        biasUpperBound--;
                    }
                    baseCoverage -= aaStep;
                }

                // Calculate using our best known formula so far
                const i32 covStartX = slope.IsNegative() ? slope.XEnd(y) : slope.XStart(y);
                u32 calcCoverage =
                    (((2 * (covStartX - left) + 1) * slope.Height() * 1024) / (slope.Width() * 2)) & 1023;
                if (gradFlip != 0) {
                    calcCoverage ^= 1023;
                }

                // Display results
                std::cout << "  " << std::setw(3) << std::right << y << "  ";
                std::cout << std::setw(3) << std::right << startX << ".." << std::setw(3) << std::left << endX
                          << " -> ";
                std::cout << std::setw(4) << std::right << calcCoverage;
                std::cout << "  ";
                if (calcCoverage >= biasLowerBound && calcCoverage <= biasUpperBound) {
                    std::cout << "  ";
                    result.numMatches++;
                } else {
                    std::cout << "!=";
                }
                result.testedPixels++;
                std::cout << "  ";
                if (biasLowerBound >= 1024) {
                    std::cout << "(unexpected gradient)";
                } else {
                    std::cout << std::setw(4) << std::right << biasLowerBound;
                    if (biasLowerBound != biasUpperBound) {
                        std::cout << ".." << std::setw(4) << std::left << biasUpperBound;
                    } else {
                        std::cout << "      ";
                    }
                }
                std::cout << "  ";
                for (i32 x = startX; slope.IsNegative() ? x >= endX : x <= endX; x += incX) {
                    std::cout << " " << std::setw(2) << std::right << (u32)line.Pixel(x, y);
                }
                std::cout << '\n';
            }
        }
    };
    calcGradient(ltSlope, ltTargetX, ltTargetY, ltStartY, ltEndY);
    calcGradient(rbSlope, rbTargetX, rbTargetY, rbStartY, rbEndY);*/

    // Generate slopes and check the coverage values
    auto calcSlope = [&](const Slope &slope, std::string slopeName, i32 testX, i32 testY, i32 startY, i32 endY) {
        for (i32 y = startY; y < endY; y++) {
            i32 startX = slope.XStart(y);
            i32 endX = slope.XEnd(y);
            i32 incX = slope.IsNegative() ? -1 : +1;

            // All tests draw a triangle with one edge covering the entire span of the screen border given by the test
            // name. Due to polygon drawing rules and edge precedences, in some cases these pixels will override the
            // tested slopes with pixels of full coverage, producing false negatives if checked blindly. The following
            // conditions skip such pixels.
            if (data.type == TEST_LEFT && startX == 0) {
                // The Left test draws a vertical line on the left side of the screen which is considered a left edge in
                // all cases, and thus overrides all pixels at X=0.
                startX++;
            }
            if ((data.type == TEST_TOP || data.type == TEST_BOTTOM) && slope.Height() == 0) {
                // The Top and bottom tests draw a horizontal line at the top or bottom of the screen. Only the leftmost
                // pixel of the left edge is valid.
                if (slope.IsNegative() == slope.IsLeftEdge()) {
                    startX = endX;
                } else {
                    endX = startX;
                }
            }
            for (i32 x = startX; slope.IsNegative() ? x >= endX : x <= endX; x += incX) {
                const i32 fracCoverage = slope.FracAACoverage(x, y);
                const i32 aaFracBits = Slope::kAAFracBits;
                const i32 coverage = fracCoverage >> aaFracBits;

                // Compare against data captured from hardware
                auto &line = data.lines[testY][testX];
                u8 pixel = line.Pixel(x, y);
                result.testedPixels++;
                if (coverage == pixel) {
                    result.numMatches++;
                } else {
                    foundMismatch();
                }
                if (coverage > pixel) {
                    result.overshoot += coverage - pixel;
                } else if (coverage < pixel) {
                    result.undershoot += pixel - coverage;
                }
                // if (coverage != pixel) {
                std::cout << std::setw(3) << std::right << testX << 'x' << std::setw(3) << std::left << testY  //
                          << " @ " << std::setw(3) << std::right << x << 'x' << std::setw(3) << std::left << y //
                          << "  " << slopeName << ": "                                                         //
                          << std::setw(2) << std::right << coverage << ((coverage == pixel) ? " == " : " != ") //
                          << std::setw(2) << (u32)pixel                                                        //
                          << "  (" << std::setw(4) << fracCoverage << "  "                                     //
                          << std::setw(2) << std::right << (fracCoverage >> aaFracBits) << '.' << std::setw(2) //
                          << std::left << (fracCoverage & ((1 << aaFracBits) - 1)) << ')'                      //
                          << "   "                                                                             //
                          << (slope.IsLeftEdge() ? 'L' : 'R')                                                  //
                          << (slope.IsPositive() ? 'P' : 'N')                                                  //
                          << (slope.IsXMajor() ? 'X' : 'Y')                                                    //
                          << '\n';
                //}
            }
        }
    };
    calcSlope(ltSlope, "LT", ltTargetX, ltTargetY, ltStartY, ltEndY);
    // calcSlope(rbSlope, "RB", rbTargetX, rbTargetY, rbStartY, rbEndY);
}

void testSlopes(Data &data, i32 x0, i32 y0, const char *name) {
    std::cout << "Testing " << name << " slopes... ";
    std::cout << '\n';

    TestResult result{};

    // All slopes
    /*for (i32 y = data.minY; y <= data.maxY; y++) {
        for (i32 x = data.minX; x <= data.maxX; x++) {
            testSlope(data, x, y, result);
        }
    }*/

    // All X-major slopes
    /*for (i32 y = data.minY; y <= data.maxY; y++) {
        for (i32 x = std::max<i32>(data.minX, y + 1); x <= data.maxX; x++) {
            testSlope(data, x, y, result);
        }
    }*/

    // All Y-major slopes (except diagonals)
    /*for (i32 y = data.minY; y <= data.maxY; y++) {
        for (i32 x = data.minX; x <= std::min<i32>(data.maxX, y - 1); x++) {
            testSlope(data, x, y, result);
        }
    }*/

    testSlope(data, 54, 2, result);
    testSlope(data, 56, 2, result);
    testSlope(data, 57, 3, result);
    testSlope(data, 60, 3, result);
    testSlope(data, 62, 3, result);
    testSlope(data, 245, 192, result);
    // testSlope(data, 186, 185, result);

    if (!result.mismatch) {
        std::cout << "OK!\n";
    }
    std::cout << "## Accuracy: " << result.numMatches << " / " << result.testedPixels << " (" << std::fixed
              << std::setprecision(2) << ((double)result.numMatches / result.testedPixels * 100.0) << "%)\n";
    std::cout << "overshoot: " << result.overshoot << "\n";
    std::cout << "undershoot: " << result.undershoot << "\n";
}

void test(Data &data) {
    switch (data.type) {
    case 0: testSlopes(data, 0, 0, "top"); break;
    case 1: testSlopes(data, 256, 0, "bottom"); break;
    case 2: testSlopes(data, 0, 192, "left"); break;
    case 3: testSlopes(data, 256, 192, "right"); break;
    }
}