#pragma once

#include "dataset.h"
#include "func.h"

#include <filesystem>
#include <vector>

void generateFunc(const std::vector<Operation> &templateOps, const std::vector<DataPoint> &dataPoints);
std::vector<Operation> generateFuncDFS(const std::vector<Operation> &templateOps, std::filesystem::path datasetRoot);
