#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "types.h"

struct XMBDataPoint {
    u16 width, height;
    u16 biasLB, biasUB;
    u8 y;

    std::string KeyStr() const {
        return std::to_string(width) + "x" + std::to_string(height) + " @ y=" + std::to_string(y);
    }
};

struct XMajorBiasDataSet {
    std::vector<XMBDataPoint> lpx;
    std::vector<XMBDataPoint> lnx;
    std::vector<XMBDataPoint> rpx;
    std::vector<XMBDataPoint> rnx;
};

void extractXMajorBiasDataSet(std::filesystem::path root);
XMajorBiasDataSet loadXMajorBiasDataSet(std::filesystem::path root);
