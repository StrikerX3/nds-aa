#include "func_generator.h"

#include "dataset.h"

#include <array>
#include <chrono>
#include <iostream>

void generateFunc(const std::vector<Operation> &templateOps, const std::vector<DataPoint> &dataPoints) {
    Evaluator eval;

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

    // NOTE: assuming left positive X-major slope data

    auto t1 = std::chrono::steady_clock::now();
    std::vector<Operation> resultOps;
    for (;;) {
        nextTemplate();
        // randomTemplate();

        bool valid = true;
        for (auto &dataPoint : dataPoints) {
            if (i32 result; eval.EvalXMajor(dataPoint, true, true, result)) {
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
            if (i32 result; eval.EvalXMajor(dataPoint, true, true, result)) {
                std::cout << dataPoint.width << "x" << dataPoint.height << " @ " << dataPoint.x << "x" << dataPoint.y
                          << "  " << result << (result == dataPoint.expectedOutput ? " == " : " != ")
                          << dataPoint.expectedOutput << "\n";
            }
        }
    } else {
        std::cout << "Could not find sequence of operations that matches the given data points\n";
    }
}

struct DFSFuncGenerator {
    const std::vector<Operation> &templateOps;
    DataSet dataSet;

    static constexpr size_t kMaxOperations = 6;
    static constexpr size_t kMaxStackSize = kMaxOperations; // current operations can add at most 1 item to the stack

    FixedStack stack;
    std::array<FixedStack, kMaxOperations> stackBackups;
    std::array<Operation, kMaxOperations> formula;
    size_t formulaLength;

    struct PrecomputedDataPoint {
        Slope slope;
        Variables vars;
        i32 expectedOutput;

        PrecomputedDataPoint(const DataPoint &dp, bool left, bool positive)
            : expectedOutput(dp.expectedOutput) {

            if (positive) {
                slope.Setup(0, 0, dp.width, dp.height, left);
            } else {
                slope.Setup(dp.width, 0, 0, dp.height, left);
            }
            vars.Apply(dp, left);
        }
    };
    std::vector<PrecomputedDataPoint> precomputedDataPoints;

    DFSFuncGenerator(const std::vector<Operation> &templateOps, std::filesystem::path datasetRoot)
        : templateOps(templateOps)
        , dataSet(loadXMajorDataSet(datasetRoot)) {
        for (auto &dataPoint : dataSet.lpx) {
            precomputedDataPoints.emplace_back(dataPoint, true, true);
        }
        for (auto &dataPoint : dataSet.lnx) {
            precomputedDataPoints.emplace_back(dataPoint, true, false);
        }
        for (auto &dataPoint : dataSet.rpx) {
            precomputedDataPoints.emplace_back(dataPoint, false, true);
        }
        for (auto &dataPoint : dataSet.rnx) {
            precomputedDataPoints.emplace_back(dataPoint, false, false);
        }
    }

    bool search() {
        formulaLength = 0;
        stack.clear();
        return search(0);
    }

    bool search(size_t level) {
        if (level == kMaxOperations) {
            return false;
        }
        if (stack.size() > 3) {
            // Can't possibly reduce the stack to 1 item at this point
            return false;
        }
        stackBackups[level] = stack; // TODO: optimize stack handling
        for (auto &op : templateOps) {
#ifdef _DEBUG
            formula[level] = op;
            printf("testing formula:");
            for (size_t i = 0; i <= level; i++) {
                auto opStr = formula[i].Str();
                printf(" %s", opStr.c_str());
            }
            printf("\n");
#endif
            bool allPass = true;
            for (auto &dp : precomputedDataPoints) {
                stack = stackBackups[level]; // TODO: optimize stack handling
                if (!op.Execute(dp.slope, stack, dp.vars) || stack.size() != 1 || stack[0] != dp.expectedOutput) {
                    allPass = false;
                    break;
                }
            }
            if (allPass || search(level + 1)) {
                formula[level] = op;
                formulaLength = std::max(formulaLength, level);
                return true;
            }
        }
        return false;
    }
};

std::vector<Operation> generateFuncDFS(const std::vector<Operation> &templateOps, std::filesystem::path datasetRoot) {
    auto dfs = std::make_unique<DFSFuncGenerator>(templateOps, datasetRoot);
    std::vector<Operation> formula;
    if (dfs->search()) {
        formula.resize(dfs->formulaLength);
        std::copy_n(dfs->formula.begin(), formula.size(), formula.begin());
    }
    return formula;
}
