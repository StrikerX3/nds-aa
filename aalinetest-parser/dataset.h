#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "types.h"

struct DataPoint {
    i32 x, y;
    i32 width, height;
    i32 expectedOutput;

    std::string KeyStr() const {
        return std::to_string(width) + "x" + std::to_string(height) + " @ " + std::to_string(x) + "x" +
               std::to_string(y);
    }
};

struct DataSet {
    std::vector<DataPoint> lpx;
    std::vector<DataPoint> lpy;
    std::vector<DataPoint> lnx;
    std::vector<DataPoint> lny;
    std::vector<DataPoint> rpx;
    std::vector<DataPoint> rpy;
    std::vector<DataPoint> rnx;
    std::vector<DataPoint> rny;
};

void extractDataSet(std::filesystem::path root);
DataSet loadDataSet(std::filesystem::path root);
DataSet loadXMajorDataSet(std::filesystem::path root);
