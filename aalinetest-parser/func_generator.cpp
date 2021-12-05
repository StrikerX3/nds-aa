#include "func_generator.h"

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
            if (i32 result; eval.EvalXMajor(dataPoint, true, true, true, result)) {
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
            if (i32 result; eval.EvalXMajor(dataPoint, true, true, true, result)) {
                std::cout << dataPoint.width << "x" << dataPoint.height << " @ " << dataPoint.x << "x" << dataPoint.y
                          << "  " << result << (result == dataPoint.expectedOutput ? " == " : " != ")
                          << dataPoint.expectedOutput << "\n";
            }
        }
    } else {
        std::cout << "Could not find sequence of operations that matches the given data points\n";
    }
}
