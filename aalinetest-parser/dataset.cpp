#include "dataset.h"

#include "file.h"
#include "slope.h"

#include <algorithm>

void extractDataSet(std::filesystem::path root) {
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

    Output outLPX{root / "LPX.bin"};
    Output outLPY{root / "LPY.bin"};
    Output outLNX{root / "LNX.bin"};
    Output outLNY{root / "LNY.bin"};
    Output outRPX{root / "RPX.bin"};
    Output outRPY{root / "RPY.bin"};
    Output outRNX{root / "RNX.bin"};
    Output outRNY{root / "RNY.bin"};

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
}

std::vector<DataPoint> loadOne(std::filesystem::path file) {
    std::ifstream in{file, std::ios::binary};
    std::vector<DataPoint> dataset;

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
            u16 x = (entry >> 0);
            u8 y = (entry >> 16);
            u8 cov = (entry >> 24);
            dataset.push_back(DataPoint{
                .x = x,
                .y = y,
                .width = width,
                .height = height,
                .expectedOutput = cov,
            });
        }
    }

    std::sort(dataset.begin(), dataset.end(), [](const DataPoint &lhs, const DataPoint &rhs) {
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

        if (lhs.x < rhs.x) {
            return true;
        }
        if (lhs.x > rhs.x) {
            return false;
        }

        return lhs.y < rhs.y;
    });

    return dataset;
}

DataSet loadDataSet(std::filesystem::path root) {
    return DataSet{.lpx = loadOne(root / "LPX.bin"),
                   .lpy = loadOne(root / "LPY.bin"),
                   .lnx = loadOne(root / "LNX.bin"),
                   .lny = loadOne(root / "LNY.bin"),
                   .rpx = loadOne(root / "RPX.bin"),
                   .rpy = loadOne(root / "RPY.bin"),
                   .rnx = loadOne(root / "RNX.bin"),
                   .rny = loadOne(root / "RNY.bin")};
}
