#include "biasdataset.h"

#include "file.h"
#include "slope.h"

#include <algorithm>

void extractXMajorBiasDataSet(std::filesystem::path root) {
    auto dataT = readFile(root / "T.bin");
    auto dataB = readFile(root / "B.bin");

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

    Output outLPX{root / "LPX-bias.bin"};
    Output outLNX{root / "LNX-bias.bin"};
    Output outRPX{root / "RPX-bias.bin"};
    Output outRNX{root / "RNX-bias.bin"};

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

            // Edge side per test:
            //           LT    RB
            // TOP      left  right
            // BOTTOM   left  right
            // LEFT     right right
            // RIGHT    left  left

            Slope tltSlope{}; // top LT
            Slope trbSlope{}; // top RB
            Slope bltSlope{}; // bottom LT
            Slope brbSlope{}; // bottom RB
            tltSlope.Setup(tltCoords.startX, tltCoords.startY, tltCoords.endX, tltCoords.endY, true);
            trbSlope.Setup(trbCoords.startX, trbCoords.startY, trbCoords.endX, trbCoords.endY, false);
            bltSlope.Setup(bltCoords.startX, bltCoords.startY, bltCoords.endX, bltCoords.endY, true);
            brbSlope.Setup(brbCoords.startX, brbCoords.startY, brbCoords.endX, brbCoords.endY, false);

            auto calcSlope = [&](i32 xOffset, i32 startY, i32 yy, const Data &data, const Slope &slope, Output &out) {
                // We're only interested in X-major slopes
                if (slope.Width() == 0 || slope.Height() == 0 || slope.Width() <= slope.Height()) {
                    return;
                }

                // Determine horizontal span
                i32 startX = slope.XStart(yy);
                i32 endX = slope.XEnd(yy);
                const bool flipped = startX > endX;
                if (flipped) {
                    std::swap(startX, endX);
                }

                const i32 gradFlip = (slope.IsLeftEdge() == slope.IsNegative()) ? 31 : 0;
                const i32 aaStep = slope.Height() * 1024 / slope.Width();

                // Determine the valid bias range
                i32 biasLowerBound = 0;
                i32 biasUpperBound = 0;
                i32 baseCoverage = 0;
                auto &line = data.lines[y][x];

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
                    while (biasLowerBound < 1024 && aaCovLower() != line.Pixel(x, yy)) {
                        biasLowerBound++;
                    }
                    baseCoverage += aaStep;
                }
                baseCoverage -= aaStep;

                // Extend upper bound as far as the last value in the gradient allows
                biasUpperBound = biasLowerBound;
                while (biasUpperBound < 1024 && aaCovUpper() == line.Pixel(endX, yy)) {
                    biasUpperBound++;
                }

                // Do a backward scan to find the upper bound
                for (i32 x = endX; x >= startX; x--) {
                    // Ignore the leftmost pixel of 256-wide right slopes because they make it impossible to find a
                    // valid bias for a gradient
                    if (slope.Width() == 256 && slope.IsRightEdge() && x == 0) {
                        continue;
                    }
                    while (biasUpperBound > 0 && aaCovUpper() != line.Pixel(x, yy)) {
                        biasUpperBound--;
                    }
                    baseCoverage -= aaStep;
                }

                /*std::cout << std::setw(3) << std::right << slope.Width() << 'x' << std::setw(3) << std::left
                          << slope.Height();
                std::cout << "   "                            //
                          << (slope.IsLeftEdge() ? 'L' : 'R') //
                          << (slope.IsPositive() ? 'P' : 'N') //
                          << (slope.IsXMajor() ? 'X' : 'Y');  //
                std::cout << "    y=" << std::setw(3) << std::left << yy << "   x=";
                std::cout << std::setw(3) << std::right << startX << ".." << std::setw(3) << std::left << endX
                          << " -> ";
                if (biasLowerBound >= 1024) {
                    // std::cout << ';';
                    std::cout << "(unexpected gradient)";
                } else {
                    // std::cout << biasLowerBound << ';' << biasUpperBound;
                    std::cout << std::setw(4) << std::right << biasLowerBound;
                    if (biasLowerBound != biasUpperBound) {
                        std::cout << ".." << std::setw(4) << std::left << biasUpperBound;
                    }
                }
                std::cout << '\n';*/

                // Write slope values from hardware capture using the generated slope
                // Data format:
                //   [u16] width
                //   [u16] height
                //   repeated:
                //     [u32 LE bitfield]
                //       [ 0..7 ] y
                //       [ 8..18] minimum bias (0..1024)
                //       [19..29] maximum bias (0..1024)
                //     (if all of these are FFs, end of list)
                const i32 w = slope.Width();
                const i32 h = slope.Height();
                out.Size(w, h);
                union {
                    u32 value;
                    struct {
                        u32 y : 8;
                        u32 biasLB : 11;
                        u32 biasUB : 11;
                    };
                } value{
                    .y = (u32)(yy - startY),
                    .biasLB = (u32)biasLowerBound,
                    .biasUB = (u32)biasUpperBound,
                };
                out.stream.write((const char *)&value.value, sizeof(value.value));
            };

            // Generate slopes and write coverage values to the corresponding data set
            for (i32 yy = 0; yy < y; yy++) {
                calcSlope(std::min(tltCoords.startX, tltCoords.endX), 0, yy, *dataT, tltSlope, outLPX);
                calcSlope(std::min(trbCoords.startX, trbCoords.endX), 0, yy, *dataT, trbSlope, outRNX);
            }
            for (i32 yy = y; yy < 192; yy++) {
                calcSlope(std::min(bltCoords.startX, bltCoords.endX), y, yy, *dataB, bltSlope, outLNX);
                calcSlope(std::min(brbCoords.startX, brbCoords.endX), y, yy, *dataB, brbSlope, outRPX);
            }
        }
    }
}

std::vector<XMBDataPoint> loadOne(std::filesystem::path file) {
    std::ifstream in{file, std::ios::binary};
    std::vector<XMBDataPoint> dataset;

    while (in) {
        u16 width, height;
        in.read((char *)&width, sizeof(u16));
        in.read((char *)&height, sizeof(u16));

        u32 entry;
        for (;;) {
            in.read((char *)&entry, sizeof(u32));
            if (entry == 0xFFFFFFFF) {
                break;
            }
            union {
                u32 value;
                struct {
                    u32 y : 8;
                    u32 biasLB : 11;
                    u32 biasUB : 11;
                };
            } value{.value = entry};
            dataset.push_back(XMBDataPoint{
                .width = width,
                .height = height,
                .biasLB = (u16)value.biasLB,
                .biasUB = (u16)value.biasUB,
                .y = (u8)value.y,
            });
        }
    }

    std::sort(dataset.begin(), dataset.end(), [](const XMBDataPoint &lhs, const XMBDataPoint &rhs) {
        if (lhs.height < rhs.height) {
            return true;
        }
        if (lhs.height > rhs.height) {
            return false;
        }

        if (lhs.width < rhs.width) {
            return true;
        }
        if (lhs.width > rhs.width) {
            return false;
        }

        return lhs.y < rhs.y;
    });

    dataset.shrink_to_fit();
    return dataset;
}

XMajorBiasDataSet loadXMajorBiasDataSet(std::filesystem::path root) {
    return XMajorBiasDataSet{
        .lpx = loadOne(root / "LPX-bias.bin"),
        .lnx = loadOne(root / "LNX-bias.bin"),
        .rpx = loadOne(root / "RPX-bias.bin"),
        .rnx = loadOne(root / "RNX-bias.bin"),
    };
}
