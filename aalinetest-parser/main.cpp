#include "biasdataset.h"
#include "dataset.h"
#include "file.h"
#include "func_generator.h"
#include "func_search.h"
#include "interactive_eval.h"
#include "tester.h"

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>

#include <bitset>
#include <chrono>
#include <iostream>
#include <memory>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

int main() {
    // convertScreenCap("data/screencap.bin", "data/screencap.tga");
    // uniqueColors("data/screencap.bin");

    auto dataT = readFile("E:/Development/_refs/NDS/Research/Antialiasing/T.bin");
    //auto dataB = readFile("E:/Development/_refs/NDS/Research/Antialiasing/B.bin");

    if (dataT)
        test(*dataT);
    //if (dataB)
    //    test(*dataB);

    // if (dataT) writeImages(*dataT, "E:/Development/_refs/NDS/Research/Antialiasing/T");
    // if (dataB) writeImages(*dataB, "E:/Development/_refs/NDS/Research/Antialiasing/B");

    return EXIT_SUCCESS;
}

// --------------------------------------------------------------------------------

int main2() {
    extractDataSet("E:/Development/_refs/NDS/Research/Antialiasing");
    return EXIT_SUCCESS;
}

// --------------------------------------------------------------------------------

int main3() {
    runInteractiveEvaluator("E:/Development/_refs/NDS/Research/Antialiasing");
    return EXIT_SUCCESS;
}

// --------------------------------------------------------------------------------

int main4() {
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
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::InsertAAFracBits});
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

    generateFunc(templateOps, dataPoints);

    return EXIT_SUCCESS;
}

// --------------------------------------------------------------------------------

int main5() {
    constexpr i32 kConstants[] = {
        -1,
        Slope::kAARange,
        Slope::kAARange - 1,
        Slope::kAAFracBits,
        Slope::kAAFracBits + 5,
        Slope::kAAFracRange,
        Slope::kAAFracRange - 1,
        Slope::kFracBits,
        Slope::kFracBits >> 1,
        Slope::kOne,
        Slope::kOne - 1,
        Slope::kBias,
    };
    std::vector<Operation> templateOps;
    {
        for (i32 b = 0; b < 31; b++) {
            templateOps.push_back(Operation{.type = Operation::Type::Constant, .constVal = (1 << b)});
            templateOps.push_back(Operation{.type = Operation::Type::Constant, .constVal = (1 << b) - 1});
        }
        for (i32 c : kConstants) {
            templateOps.push_back(Operation{.type = Operation::Type::Constant, .constVal = c});
        }
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::PushX});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::PushY});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::PushWidth});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::PushHeight});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::PushPositive});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::PushNegative});
        // templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::PushXMajor});
        // templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::PushYMajor});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::PushLeft});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::PushRight});

        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Add});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Subtract});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Multiply});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Divide});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Modulo});
        // templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Negate});
        // templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::LeftShift});
        // templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::ArithmeticRightShift});
        // templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::LogicRightShift});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::And});
        // templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Or});
        // templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Xor});
        // templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Not});

        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Dup});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Over});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Swap});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Drop});
        // templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Rot});
        // templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::RevRot});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::IfElse});

        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::FracXStart});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::FracXEnd});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::FracXWidth});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::XStart});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::XEnd});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::XWidth});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::X0});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::InsertAAFracBits});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::InvertAA});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::InvertAAFrac});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::MulWidth});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::MulHeight});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::DivWidth});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::DivHeight});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Add1});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Sub1});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Mul2});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Div2});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::MulHeightDivWidthAA});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::AAStep});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::And1});
    }

    std::vector<ExtDataPoint> dataPoints;
    /*dataPoints.push_back(ExtDataPoint{
        .dp = {.x = 36, .y = 1, .width = 54, .height = 2, .expectedOutput = 11}, .left = true, .positive = true});
    dataPoints.push_back(ExtDataPoint{
        .dp = {.x = 2, .y = 1, .width = 15, .height = 6, .expectedOutput = 0}, .left = true, .positive = true});
    dataPoints.push_back(ExtDataPoint{
        .dp = {.x = 7, .y = 3, .width = 15, .height = 6, .expectedOutput = 0}, .left = true, .positive = true});
    dataPoints.push_back(ExtDataPoint{
        .dp = {.x = 9, .y = 0, .width = 54, .height = 2, .expectedOutput = 10}, .left = true, .positive = true});
    dataPoints.push_back(ExtDataPoint{
        .dp = {.x = 12, .y = 5, .width = 15, .height = 6, .expectedOutput = 0}, .left = true, .positive = true});
    dataPoints.push_back(ExtDataPoint{
        .dp = {.x = 44, .y = 3, .width = 56, .height = 5, .expectedOutput = 31}, .left = true, .positive = true});
    dataPoints.push_back(ExtDataPoint{
        .dp = {.x = 45, .y = 4, .width = 56, .height = 5, .expectedOutput = 2}, .left = true, .positive = true});

    dataPoints.push_back(ExtDataPoint{
        .dp = {.x = 106, .y = 106, .width = 186, .height = 185, .expectedOutput = 2}, .left = true, .positive = true});
    dataPoints.push_back(ExtDataPoint{
        .dp = {.x = 107, .y = 106, .width = 186, .height = 185, .expectedOutput = 31}, .left = true, .positive = true});

    dataPoints.push_back(ExtDataPoint{
        .dp = {.x = 0, .y = 0, .width = 15, .height = 6, .expectedOutput = 6}, .left = true, .positive = true});
    dataPoints.push_back(ExtDataPoint{
        .dp = {.x = 1, .y = 0, .width = 15, .height = 6, .expectedOutput = 19}, .left = true, .positive = true});
    dataPoints.push_back(ExtDataPoint{
        .dp = {.x = 35, .y = 1, .width = 54, .height = 2, .expectedOutput = 9}, .left = true, .positive = true});
    dataPoints.push_back(ExtDataPoint{
        .dp = {.x = 3, .y = 1, .width = 15, .height = 6, .expectedOutput = 12}, .left = true, .positive = true});
    dataPoints.push_back(ExtDataPoint{
        .dp = {.x = 105, .y = 105, .width = 186, .height = 185, .expectedOutput = 29}, .left = true, .positive = true});
    dataPoints.push_back(ExtDataPoint{
        .dp = {.x = 37, .y = 1, .width = 54, .height = 2, .expectedOutput = 12}, .left = true, .positive = true});
    dataPoints.push_back(ExtDataPoint{
        .dp = {.x = 4, .y = 1, .width = 15, .height = 6, .expectedOutput = 25}, .left = true, .positive = true});
    dataPoints.push_back(ExtDataPoint{
        .dp = {.x = 43, .y = 3, .width = 56, .height = 5, .expectedOutput = 28}, .left = true, .positive = true});
    dataPoints.push_back(ExtDataPoint{
        .dp = {.x = 6, .y = 2, .width = 15, .height = 6, .expectedOutput = 19}, .left = true, .positive = true});
    dataPoints.push_back(ExtDataPoint{
        .dp = {.x = 10, .y = 4, .width = 15, .height = 6, .expectedOutput = 6}, .left = true, .positive = true});
    dataPoints.push_back(ExtDataPoint{
        .dp = {.x = 108, .y = 107, .width = 186, .height = 185, .expectedOutput = 29}, .left = true, .positive = true});
    dataPoints.push_back(ExtDataPoint{
        .dp = {.x = 8, .y = 0, .width = 54, .height = 2, .expectedOutput = 9}, .left = true, .positive = true});
    dataPoints.push_back(ExtDataPoint{
        .dp = {.x = 46, .y = 4, .width = 56, .height = 5, .expectedOutput = 4}, .left = true, .positive = true});
    dataPoints.push_back(ExtDataPoint{
        .dp = {.x = 92, .y = 92, .width = 186, .height = 185, .expectedOutput = 0}, .left = true, .positive = true});
    dataPoints.push_back(ExtDataPoint{
        .dp = {.x = 93, .y = 93, .width = 186, .height = 185, .expectedOutput = 31}, .left = true, .positive = true});

    dataPoints.push_back(ExtDataPoint{
        .dp = {.x = 5, .y = 2, .width = 15, .height = 6, .expectedOutput = 6}, .left = true, .positive = true});
    dataPoints.push_back(ExtDataPoint{
        .dp = {.x = 8, .y = 3, .width = 15, .height = 6, .expectedOutput = 12}, .left = true, .positive = true});
    dataPoints.push_back(ExtDataPoint{
        .dp = {.x = 9, .y = 3, .width = 15, .height = 6, .expectedOutput = 25}, .left = true, .positive = true});
    dataPoints.push_back(ExtDataPoint{
        .dp = {.x = 11, .y = 4, .width = 15, .height = 6, .expectedOutput = 19}, .left = true, .positive = true});
    dataPoints.push_back(ExtDataPoint{
        .dp = {.x = 13, .y = 5, .width = 15, .height = 6, .expectedOutput = 12}, .left = true, .positive = true});
    dataPoints.push_back(ExtDataPoint{
        .dp = {.x = 14, .y = 5, .width = 15, .height = 6, .expectedOutput = 25}, .left = true, .positive = true});
    dataPoints.push_back(ExtDataPoint{
        .dp = {.x = 10, .y = 0, .width = 54, .height = 2, .expectedOutput = 12}, .left = true, .positive = true});*/

    /*dataPoints.push_back({{0, 0, 205, 2, 4}, true, true});
    dataPoints.push_back({{0, 1, 205, 2, 10}, true, true});

    dataPoints.push_back({{0, 0, 206, 2, 4}, true, true});
    dataPoints.push_back({{0, 1, 206, 2, 5}, true, true});

    dataPoints.push_back({{0, 0, 207, 2, 4}, true, true});
    dataPoints.push_back({{0, 1, 207, 2, 9}, true, true});

    dataPoints.push_back({{0, 0, 208, 2, 4}, true, true});
    dataPoints.push_back({{0, 1, 208, 2, 5}, true, true});

    dataPoints.push_back({{0, 0, 209, 2, 4}, true, true});
    dataPoints.push_back({{0, 1, 209, 2, 9}, true, true});

    dataPoints.push_back({{0, 0, 210, 2, 4}, true, true});
    dataPoints.push_back({{0, 1, 210, 2, 5}, true, true});

    dataPoints.push_back({{0, 0, 211, 2, 4}, true, true});
    dataPoints.push_back({{0, 1, 211, 2, 9}, true, true});

    dataPoints.push_back({{0, 0, 212, 2, 4}, true, true});
    dataPoints.push_back({{0, 1, 212, 2, 5}, true, true});

    dataPoints.push_back({{0, 0, 213, 2, 4}, true, true});
    dataPoints.push_back({{0, 1, 213, 2, 9}, true, true});

    dataPoints.push_back({{0, 0, 214, 2, 4}, true, true});
    dataPoints.push_back({{0, 1, 214, 2, 4}, true, true});

    dataPoints.push_back({{0, 0, 215, 2, 4}, true, true});
    dataPoints.push_back({{0, 1, 215, 2, 9}, true, true});

    dataPoints.push_back({{0, 0, 216, 2, 4}, true, true});
    dataPoints.push_back({{0, 1, 216, 2, 4}, true, true});

    dataPoints.push_back({{0, 0, 217, 2, 4}, true, true});
    dataPoints.push_back({{0, 1, 217, 2, 9}, true, true});

    dataPoints.push_back({{0, 0, 218, 2, 4}, true, true});
    dataPoints.push_back({{0, 1, 218, 2, 4}, true, true});

    dataPoints.push_back({{0, 0, 219, 2, 4}, true, true});
    dataPoints.push_back({{0, 1, 219, 2, 9}, true, true});

    dataPoints.push_back({{0, 0, 220, 2, 4}, true, true});
    dataPoints.push_back({{0, 1, 220, 2, 4}, true, true});

    dataPoints.push_back({{0, 0, 221, 2, 4}, true, true});
    dataPoints.push_back({{0, 1, 221, 2, 9}, true, true});

    dataPoints.push_back({{0, 0, 222, 2, 4}, true, true});
    dataPoints.push_back({{0, 1, 222, 2, 4}, true, true});

    dataPoints.push_back({{0, 0, 223, 2, 4}, true, true});
    dataPoints.push_back({{0, 1, 223, 2, 9}, true, true});

    dataPoints.push_back({{0, 0, 224, 2, 4}, true, true});
    dataPoints.push_back({{0, 1, 224, 2, 4}, true, true});

    dataPoints.push_back({{0, 0, 225, 2, 4}, true, true});
    dataPoints.push_back({{0, 1, 225, 2, 9}, true, true});

    dataPoints.push_back({{0, 0, 226, 2, 4}, true, true});
    dataPoints.push_back({{0, 1, 226, 2, 4}, true, true});

    dataPoints.push_back({{0, 0, 227, 2, 4}, true, true});
    dataPoints.push_back({{0, 1, 227, 2, 9}, true, true});

    // ----

    dataPoints.push_back({{0, 0, 68, 3, 22}, true, true});
    dataPoints.push_back({{0, 1, 68, 3, 37}, true, true});

    dataPoints.push_back({{0, 0, 70, 3, 21}, true, true});
    dataPoints.push_back({{0, 1, 70, 3, 7}, true, true});
    dataPoints.push_back({{0, 2, 70, 3, 36}, true, true});

    dataPoints.push_back({{0, 0, 71, 3, 21}, true, true});
    dataPoints.push_back({{0, 1, 71, 3, 36}, true, true});
    dataPoints.push_back({{0, 2, 71, 3, 7}, true, true});

    dataPoints.push_back({{0, 0, 54, 2, 18}, true, true});
    dataPoints.push_back({{0, 1, 54, 2, 19}, true, true});

    dataPoints.push_back({{0, 0, 114, 2, 8}, true, true});
    dataPoints.push_back({{0, 1, 114, 2, 9}, true, true});

    dataPoints.push_back({{0, 0, 214, 3, 7}, true, true});
    dataPoints.push_back({{0, 1, 214, 3, 2}, true, true});
    dataPoints.push_back({{0, 2, 214, 3, 12}, true, true});

    dataPoints.push_back({{0, 0, 233, 3, 6}, true, true});
    dataPoints.push_back({{0, 1, 233, 3, 11}, true, true});
    dataPoints.push_back({{0, 2, 233, 3, 2}, true, true});

    dataPoints.push_back({{0, 6, 219, 9, 36}, true, true});

    dataPoints.push_back({{0, 3, 220, 9, 7}, true, true});

    dataPoints.push_back({{0, 47, 250, 102, 127}, true, true});

    dataPoints.push_back({{0, 88, 252, 142, 191}, true, true});

    // ----

    dataPoints.push_back({{0, 0, 211, 7, 16}, true, true});
    dataPoints.push_back({{0, 1, 211, 7, 12}, true, true});
    dataPoints.push_back({{0, 2, 211, 7, 7}, true, true});
    dataPoints.push_back({{0, 3, 211, 7, 2}, true, true});
    dataPoints.push_back({{0, 4, 211, 7, 31}, true, true});
    dataPoints.push_back({{0, 5, 211, 7, 26}, true, true});
    dataPoints.push_back({{0, 6, 211, 7, 22}, true, true});

    dataPoints.push_back({{0, 0, 212, 7, 16}, true, true});
    dataPoints.push_back({{0, 1, 212, 7, 7}, true, true});
    dataPoints.push_back({{0, 2, 212, 7, 31}, true, true});
    dataPoints.push_back({{0, 3, 212, 7, 21}, true, true});
    dataPoints.push_back({{0, 4, 212, 7, 12}, true, true});
    dataPoints.push_back({{0, 5, 212, 7, 2}, true, true});
    dataPoints.push_back({{0, 6, 212, 7, 26}, true, true});

    dataPoints.push_back({{0, 0, 213, 7, 16}, true, true});
    dataPoints.push_back({{0, 1, 213, 7, 2}, true, true});
    dataPoints.push_back({{0, 2, 213, 7, 21}, true, true});
    dataPoints.push_back({{0, 3, 213, 7, 7}, true, true});
    dataPoints.push_back({{0, 4, 213, 7, 26}, true, true});
    dataPoints.push_back({{0, 5, 213, 7, 12}, true, true});
    dataPoints.push_back({{0, 6, 213, 7, 31}, true, true});

    dataPoints.push_back({{0, 0, 214, 7, 16}, true, true});
    dataPoints.push_back({{0, 1, 214, 7, 31}, true, true});
    dataPoints.push_back({{0, 2, 214, 7, 12}, true, true});
    dataPoints.push_back({{0, 3, 214, 7, 26}, true, true});
    dataPoints.push_back({{0, 4, 214, 7, 7}, true, true});
    dataPoints.push_back({{0, 5, 214, 7, 21}, true, true});
    dataPoints.push_back({{0, 6, 214, 7, 2}, true, true});

    dataPoints.push_back({{0, 0, 215, 7, 16}, true, true});
    dataPoints.push_back({{0, 1, 215, 7, 26}, true, true});
    dataPoints.push_back({{0, 2, 215, 7, 2}, true, true});
    dataPoints.push_back({{0, 3, 215, 7, 11}, true, true});
    dataPoints.push_back({{0, 4, 215, 7, 21}, true, true});
    dataPoints.push_back({{0, 5, 215, 7, 31}, true, true});
    dataPoints.push_back({{0, 6, 215, 7, 7}, true, true});

    dataPoints.push_back({{0, 0, 216, 7, 16}, true, true});
    dataPoints.push_back({{0, 1, 216, 7, 21}, true, true});
    dataPoints.push_back({{0, 2, 216, 7, 26}, true, true});
    dataPoints.push_back({{0, 3, 216, 7, 30}, true, true});
    dataPoints.push_back({{0, 4, 216, 7, 2}, true, true});
    dataPoints.push_back({{0, 5, 216, 7, 7}, true, true});
    dataPoints.push_back({{0, 6, 216, 7, 11}, true, true});

    dataPoints.push_back({{0, 0, 217, 7, 16}, true, true});
    dataPoints.push_back({{0, 1, 217, 7, 16}, true, true});
    dataPoints.push_back({{0, 2, 217, 7, 16}, true, true});
    dataPoints.push_back({{0, 3, 217, 7, 16}, true, true});
    dataPoints.push_back({{0, 4, 217, 7, 16}, true, true});
    dataPoints.push_back({{0, 5, 217, 7, 16}, true, true});
    dataPoints.push_back({{0, 6, 217, 7, 16}, true, true});*/

    // Sanity check
    /*dataPoints.push_back({{0, 1, 2, 3, 5}, true, true});
    dataPoints.push_back({{0, 1, 2, 5, 7}, true, true});
    dataPoints.push_back({{0, 4, 2, 3, 20}, true, true});
    dataPoints.push_back({{5, 1, 2, 3, 0}, true, true});
    dataPoints.push_back({{2, 2, 2, 2, 6}, true, true});
    dataPoints.push_back({{2, 2, 5, 2, 12}, true, true});
    dataPoints.push_back({{2, 2, 7, 2, 16}, true, true});
    dataPoints.push_back({{1, 3, 8, 9, 50}, true, true});*/

    // Bias
    // Best so far: fitness = 1
    //  - - - push_x_start push_pos swap sub - - push_9 dup mul_width shl push_frac_x_end push_10 - - div push_4 - div
    //  swap push_18 mul_2 mul - push_x_width push_right - - mul mul push_y mul_width mul_height_div_width_aa sub - -
    //  div_2 - push_262144 - - push_frac_x_start - push_262143 mod - sub insert_aa_frac_bits add - - - - add
    //  push_frac_x_width - - - - - div -

    /*dataPoints.push_back({{0, 0, 54, 2, 18}, 18, true, true});
    dataPoints.push_back({{4, 0, 54, 2, 18}, 18, true, true});
    dataPoints.push_back({{18, 0, 54, 2, 18}, 18, true, true});
    dataPoints.push_back({{26, 0, 54, 2, 18}, 18, true, true});
    dataPoints.push_back({{27, 1, 54, 2, 19}, 19, true, true});
    dataPoints.push_back({{33, 1, 54, 2, 19}, 19, true, true});
    dataPoints.push_back({{45, 1, 54, 2, 19}, 19, true, true});
    dataPoints.push_back({{53, 1, 54, 2, 19}, 19, true, true});

    dataPoints.push_back({{255, 0, 54, 2, 18}, 18, false, false});
    dataPoints.push_back({{250, 0, 54, 2, 18}, 18, false, false});
    dataPoints.push_back({{240, 0, 54, 2, 18}, 18, false, false});
    dataPoints.push_back({{229, 0, 54, 2, 18}, 18, false, false});
    dataPoints.push_back({{228, 1, 54, 2, 18}, 18, false, false});
    dataPoints.push_back({{220, 1, 54, 2, 18}, 18, false, false});
    dataPoints.push_back({{210, 1, 54, 2, 18}, 18, false, false});
    dataPoints.push_back({{202, 1, 54, 2, 18}, 18, false, false});

    dataPoints.push_back({{0, 0, 55, 2, 18}, 18, true, true});
    dataPoints.push_back({{28, 1, 55, 2, 36}, 36, true, true});

    dataPoints.push_back({{255, 0, 55, 2, 0}, 0, false, false});
    dataPoints.push_back({{227, 1, 55, 2, 18}, 18, false, false});

    dataPoints.push_back({{0, 0, 19, 1, 26}, 26, true, true});

    dataPoints.push_back({{23, 1, 57, 3, 26}, 26, true, true});
    dataPoints.push_back({{24, 1, 57, 3, 26}, 26, true, true});
    dataPoints.push_back({{25, 1, 57, 3, 26}, 26, true, true});
    dataPoints.push_back({{26, 1, 57, 3, 26}, 26, true, true});

    dataPoints.push_back({{0, 0, 103, 3, 14}, 14, true, true});
    dataPoints.push_back({{34, 1, 103, 3, 5}, 5, true, true});
    dataPoints.push_back({{69, 2, 103, 3, 24}, 24, true, true});

    dataPoints.push_back({{0, 0, 105, 3, 14}, 14, true, true});
    dataPoints.push_back({{35, 1, 105, 3, 14}, 14, true, true});
    dataPoints.push_back({{70, 2, 105, 3, 14}, 14, true, true});

    dataPoints.push_back({{116, 17, 233, 34, 0}, 0, true, true});
    dataPoints.push_back({{123, 18, 233, 34, 22}, 22, true, true});

    dataPoints.push_back({{0, 0, 62, 2, 16}, 16, true, true});
    dataPoints.push_back({{31, 1, 62, 2, 16}, 16, true, true});

    dataPoints.push_back({{23, 1, 70, 3, 7}, 7, true, true});
    dataPoints.push_back({{47, 2, 70, 3, 36}, 36, true, true});

    dataPoints.push_back({{0, 0, 2, 1, 256}, 287, true, true});

    dataPoints.push_back({{0, 0, 7, 1, 70}, 73, true, true});

    dataPoints.push_back({{0, 0, 6, 2, 160}, 170, true, true});
    dataPoints.push_back({{3, 1, 6, 2, 160}, 170, true, true});

    dataPoints.push_back({{0, 0, 245, 192, 384}, 415, true, true});
    dataPoints.push_back({{1, 1, 245, 192, 160}, 189, true, true});
    dataPoints.push_back({{2, 1, 245, 192, 160}, 189, true, true});
    dataPoints.push_back({{3, 2, 245, 192, 736}, 767, true, true});
    dataPoints.push_back({{4, 3, 245, 192, 512}, 543, true, true});

    dataPoints.push_back({{196, 154, 245, 192, 0}, 29, true, true});
    dataPoints.push_back({{197, 154, 245, 192, 0}, 29, true, true});

    dataPoints.push_back({{9, 7, 255, 192, 128}, 156, true, true});
    dataPoints.push_back({{10, 7, 255, 192, 128}, 156, true, true});
    dataPoints.push_back({{11, 8, 255, 192, 672}, 703, true, true});
    dataPoints.push_back({{12, 9, 255, 192, 416}, 447, true, true});
    dataPoints.push_back({{13, 10, 255, 192, 160}, 188, true, true});
    dataPoints.push_back({{14, 10, 255, 192, 160}, 188, true, true});
    dataPoints.push_back({{15, 11, 255, 192, 672}, 703, true, true});
    dataPoints.push_back({{16, 12, 255, 192, 416}, 447, true, true});
    dataPoints.push_back({{17, 13, 255, 192, 160}, 188, true, true});
    dataPoints.push_back({{18, 13, 255, 192, 160}, 188, true, true});
    dataPoints.push_back({{19, 14, 255, 192, 672}, 703, true, true});
    dataPoints.push_back({{20, 15, 255, 192, 416}, 447, true, true});
    dataPoints.push_back({{21, 16, 255, 192, 192}, 220, true, true});
    dataPoints.push_back({{22, 16, 255, 192, 192}, 220, true, true});
    dataPoints.push_back({{23, 17, 255, 192, 704}, 735, true, true});
    dataPoints.push_back({{24, 18, 255, 192, 448}, 479, true, true});
    dataPoints.push_back({{25, 19, 255, 192, 192}, 220, true, true});
    dataPoints.push_back({{26, 19, 255, 192, 192}, 220, true, true});
    dataPoints.push_back({{27, 20, 255, 192, 704}, 735, true, true});
    dataPoints.push_back({{28, 21, 255, 192, 448}, 479, true, true});
    dataPoints.push_back({{29, 22, 255, 192, 192}, 220, true, true});
    dataPoints.push_back({{30, 22, 255, 192, 192}, 220, true, true});
    dataPoints.push_back({{31, 23, 255, 192, 704}, 735, true, true});
    dataPoints.push_back({{32, 24, 255, 192, 480}, 511, true, true});
    dataPoints.push_back({{33, 25, 255, 192, 224}, 255, true, true});
    dataPoints.push_back({{200, 151, 255, 192, 32}, 60, true, true});
    dataPoints.push_back({{201, 151, 255, 192, 32}, 60, true, true});
    dataPoints.push_back({{244, 184, 255, 192, 96}, 124, true, true});
    dataPoints.push_back({{245, 184, 255, 192, 96}, 124, true, true});

    dataPoints.push_back({{102, 102, 186, 185, 960}, 991, true, true});
    dataPoints.push_back({{103, 103, 186, 185, 960}, 991, true, true});
    dataPoints.push_back({{104, 104, 186, 185, 928}, 959, true, true});
    dataPoints.push_back({{105, 105, 186, 185, 64}, 95, true, true});
    dataPoints.push_back({{106, 106, 186, 185, 64}, 95, true, true});
    dataPoints.push_back({{107, 106, 186, 185, 928}, 959, true, true});
    dataPoints.push_back({{108, 107, 186, 185, 928}, 959, true, true});
    dataPoints.push_back({{109, 108, 186, 185, 896}, 927, true, true});
    dataPoints.push_back({{175, 174, 186, 185, 544}, 575, true, true});
    dataPoints.push_back({{176, 175, 186, 185, 544}, 575, true, true});
    dataPoints.push_back({{177, 176, 186, 185, 544}, 575, true, true});
    dataPoints.push_back({{178, 177, 186, 185, 544}, 575, true, true});
    dataPoints.push_back({{179, 178, 186, 185, 512}, 543, true, true});
    dataPoints.push_back({{180, 179, 186, 185, 512}, 543, true, true});
    dataPoints.push_back({{181, 180, 186, 185, 512}, 543, true, true});
    dataPoints.push_back({{182, 181, 186, 185, 512}, 543, true, true});
    dataPoints.push_back({{183, 182, 186, 185, 512}, 543, true, true});
    dataPoints.push_back({{184, 183, 186, 185, 512}, 543, true, true});
    dataPoints.push_back({{185, 184, 186, 185, 512}, 543, true, true});

    // dataPoints.push_back({{3, 0, 150, 12, 39}, 40, true, true});
    dataPoints.push_back({{4, 0, 150, 12, 39}, 40, true, true});
    // dataPoints.push_back({{5, 0, 150, 12, 39}, 40, true, true});
    // dataPoints.push_back({{16, 1, 150, 12, 0}, 4, true, true});
    dataPoints.push_back({{17, 1, 150, 12, 0}, 4, true, true});
    // dataPoints.push_back({{18, 1, 150, 12, 0}, 4, true, true});
    // dataPoints.push_back({{31, 2, 150, 12, 41}, 42, true, true});
    dataPoints.push_back({{32, 2, 150, 12, 41}, 42, true, true});
    // dataPoints.push_back({{33, 2, 150, 12, 41}, 42, true, true});
    // dataPoints.push_back({{42, 3, 150, 12, 0}, 4, true, true});
    dataPoints.push_back({{43, 3, 150, 12, 0}, 4, true, true});
    // dataPoints.push_back({{44, 3, 150, 12, 0}, 4, true, true});
    // dataPoints.push_back({{56, 4, 150, 12, 41}, 42, true, true});
    dataPoints.push_back({{57, 4, 150, 12, 41}, 42, true, true});
    // dataPoints.push_back({{58, 4, 150, 12, 41}, 42, true, true});
    // dataPoints.push_back({{70, 5, 150, 12, 0}, 4, true, true});
    dataPoints.push_back({{71, 5, 150, 12, 0}, 4, true, true});
    // dataPoints.push_back({{72, 5, 150, 12, 0}, 4, true, true});
    // dataPoints.push_back({{81, 6, 150, 12, 41}, 42, true, true});
    dataPoints.push_back({{82, 6, 150, 12, 41}, 42, true, true});
    // dataPoints.push_back({{83, 6, 150, 12, 41}, 42, true, true});
    // dataPoints.push_back({{91, 7, 150, 12, 0}, 4, true, true});
    dataPoints.push_back({{92, 7, 150, 12, 0}, 4, true, true});
    // dataPoints.push_back({{93, 7, 150, 12, 0}, 4, true, true});
    // dataPoints.push_back({{106, 8, 150, 12, 41}, 42, true, true});
    dataPoints.push_back({{107, 8, 150, 12, 41}, 42, true, true});
    // dataPoints.push_back({{108, 8, 150, 12, 41}, 42, true, true});
    // dataPoints.push_back({{116, 9, 150, 12, 0}, 4, true, true});
    dataPoints.push_back({{117, 9, 150, 12, 0}, 4, true, true});
    // dataPoints.push_back({{118, 9, 150, 12, 0}, 4, true, true});
    // dataPoints.push_back({{131, 10, 150, 12, 41}, 42, true, true});
    dataPoints.push_back({{132, 10, 150, 12, 41}, 42, true, true});
    // dataPoints.push_back({{133, 10, 150, 12, 41}, 42, true, true});
    // dataPoints.push_back({{143, 11, 150, 12, 0}, 4, true, true});
    dataPoints.push_back({{144, 11, 150, 12, 0}, 4, true, true});
    // dataPoints.push_back({{145, 11, 150, 12, 0}, 4, true, true});*/

    /*dataPoints.push_back({{102, 102, 186, 185, 960}, 991, true, true});
    dataPoints.push_back({{103, 103, 186, 185, 960}, 991, true, true});
    dataPoints.push_back({{104, 104, 186, 185, 928}, 959, true, true});
    //dataPoints.push_back({{105, 105, 186, 185, 64}, 95, true, true});
    //dataPoints.push_back({{106, 106, 186, 185, 64}, 95, true, true});
    dataPoints.push_back({{107, 106, 186, 185, 928}, 959, true, true});
    dataPoints.push_back({{108, 107, 186, 185, 928}, 959, true, true});
    dataPoints.push_back({{109, 108, 186, 185, 896}, 927, true, true});
    dataPoints.push_back({{175, 174, 186, 185, 544}, 575, true, true});
    dataPoints.push_back({{176, 175, 186, 185, 544}, 575, true, true});
    dataPoints.push_back({{177, 176, 186, 185, 544}, 575, true, true});
    dataPoints.push_back({{178, 177, 186, 185, 544}, 575, true, true});
    dataPoints.push_back({{179, 178, 186, 185, 512}, 543, true, true});
    dataPoints.push_back({{180, 179, 186, 185, 512}, 543, true, true});
    dataPoints.push_back({{181, 180, 186, 185, 512}, 543, true, true});
    dataPoints.push_back({{182, 181, 186, 185, 512}, 543, true, true});
    dataPoints.push_back({{183, 182, 186, 185, 512}, 543, true, true});
    dataPoints.push_back({{184, 183, 186, 185, 512}, 543, true, true});
    dataPoints.push_back({{185, 184, 186, 185, 512}, 543, true, true});

    dataPoints.push_back({{153, 102, 186, 185, 960}, 991, false, false});
    dataPoints.push_back({{152, 103, 186, 185, 960}, 991, false, false});
    dataPoints.push_back({{151, 104, 186, 185, 928}, 959, false, false});
    dataPoints.push_back({{150, 105, 186, 185, 928}, 959, false, false});
    //dataPoints.push_back({{149, 106, 186, 185, 64}, 95, false, false});
    //dataPoints.push_back({{148, 106, 186, 185, 64}, 95, false, false});
    dataPoints.push_back({{147, 107, 186, 185, 928}, 959, false, false});
    dataPoints.push_back({{146, 108, 186, 185, 928}, 959, false, false});
    dataPoints.push_back({{145, 109, 186, 185, 896}, 927, false, false});
    dataPoints.push_back({{80, 174, 186, 185, 544}, 575, false, false});
    dataPoints.push_back({{79, 175, 186, 185, 544}, 575, false, false});
    dataPoints.push_back({{78, 176, 186, 185, 544}, 575, false, false});
    dataPoints.push_back({{77, 177, 186, 185, 544}, 575, false, false});
    dataPoints.push_back({{76, 178, 186, 185, 544}, 575, false, false});
    dataPoints.push_back({{75, 179, 186, 185, 512}, 543, false, false});
    dataPoints.push_back({{74, 180, 186, 185, 512}, 543, false, false});
    dataPoints.push_back({{73, 181, 186, 185, 512}, 543, false, false});
    dataPoints.push_back({{72, 182, 186, 185, 512}, 543, false, false});
    dataPoints.push_back({{71, 183, 186, 185, 512}, 543, false, false});
    dataPoints.push_back({{70, 184, 186, 185, 512}, 543, false, false});*/

    /*dataPoints.push_back({{0, 0, 54, 2, 18}, 18, true, true});
    dataPoints.push_back({{4, 0, 54, 2, 18}, 18, true, true});
    dataPoints.push_back({{18, 0, 54, 2, 18}, 18, true, true});
    dataPoints.push_back({{26, 0, 54, 2, 18}, 18, true, true});
    dataPoints.push_back({{27, 1, 54, 2, 19}, 20, true, true});
    dataPoints.push_back({{33, 1, 54, 2, 19}, 20, true, true});
    dataPoints.push_back({{45, 1, 54, 2, 19}, 20, true, true});
    dataPoints.push_back({{53, 1, 54, 2, 19}, 20, true, true});

    dataPoints.push_back({{255, 0, 54, 2, 18}, 18, false, false});
    dataPoints.push_back({{250, 0, 54, 2, 18}, 18, false, false});
    dataPoints.push_back({{240, 0, 54, 2, 18}, 18, false, false});
    dataPoints.push_back({{229, 0, 54, 2, 18}, 18, false, false});
    dataPoints.push_back({{228, 1, 54, 2, 18}, 18, false, false});
    dataPoints.push_back({{220, 1, 54, 2, 18}, 18, false, false});
    dataPoints.push_back({{210, 1, 54, 2, 18}, 18, false, false});
    dataPoints.push_back({{202, 1, 54, 2, 18}, 18, false, false});

    dataPoints.push_back({{0, 0, 55, 2, 18}, 18, true, true});
    dataPoints.push_back({{28, 1, 55, 2, 36}, 37, true, true});

    dataPoints.push_back({{255, 0, 55, 2, 0}, 0, false, false});
    dataPoints.push_back({{227, 1, 55, 2, 18}, 18, false, false});

    dataPoints.push_back({{0, 0, 19, 1, 26}, 26, true, true});

    dataPoints.push_back({{23, 1, 57, 3, 26}, 26, true, true});
    dataPoints.push_back({{24, 1, 57, 3, 26}, 26, true, true});
    dataPoints.push_back({{25, 1, 57, 3, 26}, 26, true, true});
    dataPoints.push_back({{26, 1, 57, 3, 26}, 26, true, true});

    dataPoints.push_back({{0, 0, 103, 3, 14}, 14, true, true});
    dataPoints.push_back({{34, 1, 103, 3, 5}, 5, true, true});
    dataPoints.push_back({{69, 2, 103, 3, 24}, 24, true, true});

    dataPoints.push_back({{255, 0, 103, 3, 24}, 24, false, false});
    dataPoints.push_back({{221, 1, 103, 3, 4}, 4, false, false});
    dataPoints.push_back({{186, 2, 103, 3, 14}, 14, false, false});

    dataPoints.push_back({{0, 0, 105, 3, 14}, 14, true, true});
    dataPoints.push_back({{35, 1, 105, 3, 14}, 14, true, true});
    dataPoints.push_back({{70, 2, 105, 3, 14}, 14, true, true});

    dataPoints.push_back({{116, 17, 233, 34, 0}, 0, true, true});
    dataPoints.push_back({{123, 18, 233, 34, 22}, 22, true, true});

    dataPoints.push_back({{139, 17, 233, 34, 119}, 127, false, false});
    dataPoints.push_back({{132, 18, 233, 34, 98}, 106, false, false});

    dataPoints.push_back({{0, 0, 62, 2, 16}, 16, true, true});
    dataPoints.push_back({{31, 1, 62, 2, 16}, 16, true, true});

    dataPoints.push_back({{23, 1, 70, 3, 7}, 7, true, true});
    dataPoints.push_back({{47, 2, 70, 3, 36}, 36, true, true});

    dataPoints.push_back({{0, 0, 2, 1, 256}, 287, true, true});

    dataPoints.push_back({{0, 0, 7, 1, 70}, 73, true, true});

    dataPoints.push_back({{0, 0, 6, 2, 160}, 170, true, true});
    dataPoints.push_back({{3, 1, 6, 2, 160}, 170, true, true});*/

    dataPoints.push_back({{0, 0, 186, 185, 480}, 511, true, true});
    dataPoints.push_back({{1, 1, 186, 185, 480}, 511, true, true});
    dataPoints.push_back({{2, 2, 186, 185, 480}, 511, true, true});
    dataPoints.push_back({{3, 3, 186, 185, 480}, 511, true, true});
    dataPoints.push_back({{4, 4, 186, 185, 480}, 511, true, true});
    dataPoints.push_back({{5, 5, 186, 185, 480}, 511, true, true});
    dataPoints.push_back({{6, 6, 186, 185, 448}, 479, true, true});
    dataPoints.push_back({{7, 7, 186, 185, 448}, 479, true, true});
    dataPoints.push_back({{8, 8, 186, 185, 448}, 479, true, true});
    dataPoints.push_back({{9, 9, 186, 185, 448}, 479, true, true});
    dataPoints.push_back({{10, 10, 186, 185, 448}, 479, true, true});
    dataPoints.push_back({{11, 11, 186, 185, 448}, 479, true, true});
    dataPoints.push_back({{12, 12, 186, 185, 416}, 447, true, true});
    dataPoints.push_back({{13, 13, 186, 185, 416}, 447, true, true});
    dataPoints.push_back({{14, 14, 186, 185, 416}, 447, true, true});
    dataPoints.push_back({{15, 15, 186, 185, 416}, 447, true, true});
    dataPoints.push_back({{16, 16, 186, 185, 416}, 447, true, true});
    dataPoints.push_back({{17, 17, 186, 185, 384}, 415, true, true});
    dataPoints.push_back({{18, 18, 186, 185, 384}, 415, true, true});
    dataPoints.push_back({{19, 19, 186, 185, 384}, 415, true, true});
    dataPoints.push_back({{20, 20, 186, 185, 384}, 415, true, true});
    dataPoints.push_back({{21, 21, 186, 185, 384}, 415, true, true});
    dataPoints.push_back({{22, 22, 186, 185, 384}, 415, true, true});
    dataPoints.push_back({{23, 23, 186, 185, 352}, 383, true, true});
    dataPoints.push_back({{24, 24, 186, 185, 352}, 383, true, true});
    dataPoints.push_back({{25, 25, 186, 185, 352}, 383, true, true});
    dataPoints.push_back({{26, 26, 186, 185, 352}, 383, true, true});
    dataPoints.push_back({{27, 27, 186, 185, 352}, 383, true, true});
    dataPoints.push_back({{28, 28, 186, 185, 352}, 383, true, true});
    dataPoints.push_back({{29, 29, 186, 185, 320}, 351, true, true});
    dataPoints.push_back({{30, 30, 186, 185, 320}, 351, true, true});
    dataPoints.push_back({{31, 31, 186, 185, 320}, 351, true, true});
    dataPoints.push_back({{32, 32, 186, 185, 320}, 351, true, true});
    dataPoints.push_back({{33, 33, 186, 185, 320}, 351, true, true});
    dataPoints.push_back({{34, 34, 186, 185, 320}, 351, true, true});
    dataPoints.push_back({{35, 35, 186, 185, 288}, 319, true, true});
    dataPoints.push_back({{36, 36, 186, 185, 288}, 319, true, true});
    dataPoints.push_back({{37, 37, 186, 185, 288}, 319, true, true});
    dataPoints.push_back({{38, 38, 186, 185, 288}, 319, true, true});
    dataPoints.push_back({{39, 39, 186, 185, 288}, 319, true, true});
    dataPoints.push_back({{40, 40, 186, 185, 288}, 319, true, true});
    dataPoints.push_back({{41, 41, 186, 185, 256}, 287, true, true});
    dataPoints.push_back({{42, 42, 186, 185, 256}, 287, true, true});
    dataPoints.push_back({{43, 43, 186, 185, 256}, 287, true, true});
    dataPoints.push_back({{44, 44, 186, 185, 256}, 287, true, true});
    dataPoints.push_back({{45, 45, 186, 185, 256}, 287, true, true});
    dataPoints.push_back({{46, 46, 186, 185, 256}, 287, true, true});
    dataPoints.push_back({{47, 47, 186, 185, 224}, 255, true, true});
    dataPoints.push_back({{48, 48, 186, 185, 224}, 255, true, true});
    dataPoints.push_back({{49, 49, 186, 185, 224}, 255, true, true});
    dataPoints.push_back({{50, 50, 186, 185, 224}, 255, true, true});
    dataPoints.push_back({{51, 51, 186, 185, 224}, 255, true, true});
    dataPoints.push_back({{52, 52, 186, 185, 192}, 223, true, true});
    dataPoints.push_back({{53, 53, 186, 185, 192}, 223, true, true});
    dataPoints.push_back({{54, 54, 186, 185, 192}, 223, true, true});
    dataPoints.push_back({{55, 55, 186, 185, 192}, 223, true, true});
    dataPoints.push_back({{56, 56, 186, 185, 192}, 223, true, true});
    dataPoints.push_back({{57, 57, 186, 185, 192}, 223, true, true});
    dataPoints.push_back({{58, 58, 186, 185, 160}, 191, true, true});
    dataPoints.push_back({{59, 59, 186, 185, 160}, 191, true, true});
    dataPoints.push_back({{60, 60, 186, 185, 160}, 191, true, true});
    dataPoints.push_back({{61, 61, 186, 185, 160}, 191, true, true});
    dataPoints.push_back({{62, 62, 186, 185, 160}, 191, true, true});
    dataPoints.push_back({{63, 63, 186, 185, 160}, 191, true, true});
    dataPoints.push_back({{64, 64, 186, 185, 128}, 159, true, true});
    dataPoints.push_back({{65, 65, 186, 185, 128}, 159, true, true});
    dataPoints.push_back({{66, 66, 186, 185, 128}, 159, true, true});
    dataPoints.push_back({{67, 67, 186, 185, 128}, 159, true, true});
    dataPoints.push_back({{68, 68, 186, 185, 128}, 159, true, true});
    dataPoints.push_back({{69, 69, 186, 185, 128}, 159, true, true});
    dataPoints.push_back({{70, 70, 186, 185, 96}, 127, true, true});
    dataPoints.push_back({{71, 71, 186, 185, 96}, 127, true, true});
    dataPoints.push_back({{72, 72, 186, 185, 96}, 127, true, true});
    dataPoints.push_back({{73, 73, 186, 185, 96}, 127, true, true});
    dataPoints.push_back({{74, 74, 186, 185, 96}, 127, true, true});
    dataPoints.push_back({{75, 75, 186, 185, 96}, 127, true, true});
    dataPoints.push_back({{76, 76, 186, 185, 64}, 95, true, true});
    dataPoints.push_back({{77, 77, 186, 185, 64}, 95, true, true});
    dataPoints.push_back({{78, 78, 186, 185, 64}, 95, true, true});
    dataPoints.push_back({{79, 79, 186, 185, 64}, 95, true, true});
    dataPoints.push_back({{80, 80, 186, 185, 64}, 95, true, true});
    dataPoints.push_back({{81, 81, 186, 185, 32}, 63, true, true});
    dataPoints.push_back({{82, 82, 186, 185, 32}, 63, true, true});
    dataPoints.push_back({{83, 83, 186, 185, 32}, 63, true, true});
    dataPoints.push_back({{84, 84, 186, 185, 32}, 63, true, true});
    dataPoints.push_back({{85, 85, 186, 185, 32}, 63, true, true});
    dataPoints.push_back({{86, 86, 186, 185, 32}, 63, true, true});
    dataPoints.push_back({{87, 87, 186, 185, 0}, 31, true, true});
    dataPoints.push_back({{88, 88, 186, 185, 0}, 31, true, true});
    dataPoints.push_back({{89, 89, 186, 185, 0}, 31, true, true});
    dataPoints.push_back({{90, 90, 186, 185, 0}, 31, true, true});
    dataPoints.push_back({{91, 91, 186, 185, 0}, 31, true, true});
    dataPoints.push_back({{92, 92, 186, 185, 0}, 31, true, true});
    dataPoints.push_back({{93, 93, 186, 185, 992}, 1024, true, true});
    dataPoints.push_back({{94, 94, 186, 185, 992}, 1024, true, true});
    dataPoints.push_back({{95, 95, 186, 185, 992}, 1024, true, true});
    dataPoints.push_back({{96, 96, 186, 185, 992}, 1024, true, true});
    dataPoints.push_back({{97, 97, 186, 185, 992}, 1024, true, true});
    dataPoints.push_back({{98, 98, 186, 185, 992}, 1024, true, true});
    dataPoints.push_back({{99, 99, 186, 185, 960}, 991, true, true});
    dataPoints.push_back({{100, 100, 186, 185, 960}, 991, true, true});
    dataPoints.push_back({{101, 101, 186, 185, 960}, 991, true, true});
    dataPoints.push_back({{102, 102, 186, 185, 960}, 991, true, true});
    dataPoints.push_back({{103, 103, 186, 185, 960}, 991, true, true});
    dataPoints.push_back({{104, 104, 186, 185, 960}, 991, true, true});
    dataPoints.push_back({{105, 105, 186, 185, 928}, 959, true, true});
    dataPoints.push_back({{106, 106, 186, 185, 64}, 95, true, true, 100});
    dataPoints.push_back({{107, 108, 186, 185, 928}, 959, true, true});
    dataPoints.push_back({{108, 109, 186, 185, 928}, 959, true, true});
    dataPoints.push_back({{109, 110, 186, 185, 896}, 927, true, true});
    dataPoints.push_back({{110, 111, 186, 185, 896}, 927, true, true});
    dataPoints.push_back({{111, 112, 186, 185, 896}, 927, true, true});
    dataPoints.push_back({{112, 113, 186, 185, 896}, 927, true, true});
    dataPoints.push_back({{113, 114, 186, 185, 896}, 927, true, true});
    dataPoints.push_back({{114, 115, 186, 185, 896}, 927, true, true});
    dataPoints.push_back({{115, 116, 186, 185, 864}, 895, true, true});
    dataPoints.push_back({{116, 117, 186, 185, 864}, 895, true, true});
    dataPoints.push_back({{117, 118, 186, 185, 864}, 895, true, true});
    dataPoints.push_back({{118, 119, 186, 185, 864}, 895, true, true});
    dataPoints.push_back({{119, 120, 186, 185, 864}, 895, true, true});
    dataPoints.push_back({{120, 121, 186, 185, 864}, 895, true, true});
    dataPoints.push_back({{121, 122, 186, 185, 832}, 863, true, true});
    dataPoints.push_back({{122, 123, 186, 185, 832}, 863, true, true});
    dataPoints.push_back({{123, 124, 186, 185, 832}, 863, true, true});
    dataPoints.push_back({{124, 125, 186, 185, 832}, 863, true, true});
    dataPoints.push_back({{125, 126, 186, 185, 832}, 863, true, true});
    dataPoints.push_back({{126, 127, 186, 185, 832}, 863, true, true});
    dataPoints.push_back({{127, 128, 186, 185, 800}, 831, true, true});
    dataPoints.push_back({{128, 129, 186, 185, 800}, 831, true, true});
    dataPoints.push_back({{129, 130, 186, 185, 800}, 831, true, true});
    dataPoints.push_back({{130, 131, 186, 185, 800}, 831, true, true});
    dataPoints.push_back({{131, 132, 186, 185, 800}, 831, true, true});
    dataPoints.push_back({{132, 133, 186, 185, 800}, 831, true, true});
    dataPoints.push_back({{133, 134, 186, 185, 768}, 799, true, true});
    dataPoints.push_back({{134, 135, 186, 185, 768}, 799, true, true});
    dataPoints.push_back({{135, 136, 186, 185, 768}, 799, true, true});
    dataPoints.push_back({{136, 137, 186, 185, 768}, 799, true, true});
    dataPoints.push_back({{137, 138, 186, 185, 768}, 799, true, true});
    dataPoints.push_back({{138, 139, 186, 185, 768}, 799, true, true});
    dataPoints.push_back({{139, 140, 186, 185, 736}, 767, true, true});
    dataPoints.push_back({{140, 141, 186, 185, 736}, 767, true, true});
    dataPoints.push_back({{141, 142, 186, 185, 736}, 767, true, true});
    dataPoints.push_back({{142, 143, 186, 185, 736}, 767, true, true});
    dataPoints.push_back({{143, 144, 186, 185, 736}, 767, true, true});
    dataPoints.push_back({{144, 145, 186, 185, 704}, 735, true, true});
    dataPoints.push_back({{145, 146, 186, 185, 704}, 735, true, true});
    dataPoints.push_back({{146, 147, 186, 185, 704}, 735, true, true});
    dataPoints.push_back({{147, 148, 186, 185, 704}, 735, true, true});
    dataPoints.push_back({{148, 149, 186, 185, 704}, 735, true, true});
    dataPoints.push_back({{149, 150, 186, 185, 704}, 735, true, true});
    dataPoints.push_back({{150, 151, 186, 185, 672}, 703, true, true});
    dataPoints.push_back({{151, 152, 186, 185, 672}, 703, true, true});
    dataPoints.push_back({{152, 153, 186, 185, 672}, 703, true, true});
    dataPoints.push_back({{153, 154, 186, 185, 672}, 703, true, true});
    dataPoints.push_back({{154, 155, 186, 185, 672}, 703, true, true});
    dataPoints.push_back({{155, 156, 186, 185, 672}, 703, true, true});
    dataPoints.push_back({{156, 157, 186, 185, 640}, 671, true, true});
    dataPoints.push_back({{157, 158, 186, 185, 640}, 671, true, true});
    dataPoints.push_back({{158, 159, 186, 185, 640}, 671, true, true});
    dataPoints.push_back({{159, 160, 186, 185, 640}, 671, true, true});
    dataPoints.push_back({{160, 161, 186, 185, 640}, 671, true, true});
    dataPoints.push_back({{161, 162, 186, 185, 640}, 671, true, true});
    dataPoints.push_back({{162, 163, 186, 185, 608}, 639, true, true});
    dataPoints.push_back({{163, 164, 186, 185, 608}, 639, true, true});
    dataPoints.push_back({{164, 165, 186, 185, 608}, 639, true, true});
    dataPoints.push_back({{165, 166, 186, 185, 608}, 639, true, true});
    dataPoints.push_back({{166, 167, 186, 185, 608}, 639, true, true});
    dataPoints.push_back({{167, 168, 186, 185, 608}, 639, true, true});
    dataPoints.push_back({{168, 169, 186, 185, 576}, 607, true, true});
    dataPoints.push_back({{169, 170, 186, 185, 576}, 607, true, true});
    dataPoints.push_back({{170, 171, 186, 185, 576}, 607, true, true});
    dataPoints.push_back({{171, 172, 186, 185, 576}, 607, true, true});
    dataPoints.push_back({{172, 173, 186, 185, 576}, 607, true, true});
    dataPoints.push_back({{173, 174, 186, 185, 544}, 575, true, true});
    dataPoints.push_back({{174, 175, 186, 185, 544}, 575, true, true});
    dataPoints.push_back({{175, 176, 186, 185, 544}, 575, true, true});
    dataPoints.push_back({{176, 177, 186, 185, 544}, 575, true, true});
    dataPoints.push_back({{177, 178, 186, 185, 544}, 575, true, true});
    dataPoints.push_back({{178, 179, 186, 185, 544}, 575, true, true});
    dataPoints.push_back({{179, 180, 186, 185, 512}, 543, true, true});
    dataPoints.push_back({{180, 181, 186, 185, 512}, 543, true, true});
    dataPoints.push_back({{181, 182, 186, 185, 512}, 543, true, true});
    dataPoints.push_back({{182, 183, 186, 185, 512}, 543, true, true});
    dataPoints.push_back({{183, 184, 186, 185, 512}, 543, true, true});
    dataPoints.push_back({{184, 185, 186, 185, 512}, 543, true, true});

    // Off-by-one errors (0 or negative=ok, positive=off-by-one)
    /*constexpr auto minValue = std::numeric_limits<i32>::min();
    constexpr auto maxValue = std::numeric_limits<i32>::max();
    dataPoints.push_back({{0, 0, 114, 2, minValue}, 0, true, true});
    dataPoints.push_back({{0, 0, 147, 2, minValue}, 0, true, true});
    dataPoints.push_back({{0, 0, 54, 2, minValue}, 0, true, true});
    dataPoints.push_back({{0, 0, 57, 3, minValue}, 0, true, true});
    dataPoints.push_back({{101, 101, 191, 190, 1}, maxValue, true, true});
    dataPoints.push_back({{103, 1, 205, 2, 1}, maxValue, true, true});
    dataPoints.push_back({{103, 1, 206, 2, 1}, maxValue, true, true});
    dataPoints.push_back({{103, 2, 205, 4, 1}, maxValue, true, true});
    dataPoints.push_back({{103, 2, 206, 4, 1}, maxValue, true, true});
    dataPoints.push_back({{103, 3, 171, 5, 1}, maxValue, true, true});
    dataPoints.push_back({{103, 4, 129, 5, 1}, maxValue, true, true});
    dataPoints.push_back({{104, 1, 208, 2, 1}, maxValue, true, true});
    dataPoints.push_back({{105, 1, 210, 2, 1}, maxValue, true, true});
    dataPoints.push_back({{105, 91, 221, 192, 1}, maxValue, true, true});
    dataPoints.push_back({{106, 1, 212, 2, 1}, maxValue, true, true});
    dataPoints.push_back({{106, 102, 199, 191, 1}, maxValue, true, true});
    dataPoints.push_back({{106, 4, 133, 5, 1}, maxValue, true, true});
    dataPoints.push_back({{107, 81, 253, 191, 1}, maxValue, true, true});
    dataPoints.push_back({{108, 4, 135, 5, 1}, maxValue, true, true});
    dataPoints.push_back({{109, 1, 147, 2, minValue}, 0, false, false});
    dataPoints.push_back({{110, 3, 147, 4, 1}, maxValue, true, true});
    dataPoints.push_back({{111, 92, 227, 188, 1}, maxValue, true, true});
    dataPoints.push_back({{114, 100, 217, 191, 1}, maxValue, true, true});
    dataPoints.push_back({{114, 2, 171, 3, 1}, maxValue, true, true});
    dataPoints.push_back({{114, 2, 228, 4, 1}, maxValue, true, true});
    dataPoints.push_back({{115, 2, 173, 3, 1}, maxValue, true, true});
    dataPoints.push_back({{115, 2, 230, 4, 1}, maxValue, true, true});
    dataPoints.push_back({{115, 92, 241, 192, 1}, maxValue, true, true});
    dataPoints.push_back({{119, 3, 158, 4, 1}, maxValue, true, true});
    dataPoints.push_back({{123, 4, 154, 5, 1}, maxValue, true, true});
    dataPoints.push_back({{126, 99, 240, 188, 1}, maxValue, true, true});
    dataPoints.push_back({{128, 3, 171, 4, 1}, maxValue, true, true});
    dataPoints.push_back({{133, 119, 215, 192, 1}, maxValue, true, true});
    dataPoints.push_back({{133, 3, 222, 5, 1}, maxValue, true, true});
    dataPoints.push_back({{137, 4, 171, 5, 1}, maxValue, true, true});
    dataPoints.push_back({{138, 112, 231, 188, 1}, maxValue, true, true});
    dataPoints.push_back({{138, 4, 173, 5, 1}, maxValue, true, true});
    dataPoints.push_back({{139, 3, 231, 5, 1}, maxValue, true, true});
    dataPoints.push_back({{141, 3, 188, 4, 1}, maxValue, true, true});
    dataPoints.push_back({{141, 3, 235, 5, 1}, maxValue, true, true});
    dataPoints.push_back({{142, 1, 114, 2, minValue}, 0, false, false});
    dataPoints.push_back({{142, 89, 161, 101, minValue}, 0, true, true});
    dataPoints.push_back({{143, 2, 214, 3, 1}, maxValue, true, true});
    dataPoints.push_back({{143, 90, 161, 101, minValue}, 0, true, true});
    dataPoints.push_back({{144, 115, 236, 188, 1}, maxValue, true, true});
    dataPoints.push_back({{145, 2, 217, 3, 1}, maxValue, true, true});
    dataPoints.push_back({{145, 3, 193, 4, 1}, maxValue, true, true});
    dataPoints.push_back({{145, 4, 181, 5, 1}, maxValue, true, true});
    dataPoints.push_back({{145, 91, 161, 101, minValue}, 0, true, true});
    dataPoints.push_back({{147, 92, 161, 101, 1}, maxValue, true, true});
    dataPoints.push_back({{148, 94, 161, 101, minValue}, 0, true, true});
    dataPoints.push_back({{150, 95, 161, 101, minValue}, 0, true, true});
    dataPoints.push_back({{151, 122, 238, 192, 1}, maxValue, true, true});
    dataPoints.push_back({{151, 96, 161, 101, minValue}, 0, true, true});
    dataPoints.push_back({{154, 3, 205, 4, 1}, maxValue, true, true});
    dataPoints.push_back({{155, 3, 206, 4, 1}, maxValue, true, true});
    dataPoints.push_back({{155, 3, 207, 4, 1}, maxValue, true, true});
    dataPoints.push_back({{155, 4, 194, 5, 1}, maxValue, true, true});
    dataPoints.push_back({{156, 124, 241, 191, 1}, maxValue, true, true});
    dataPoints.push_back({{156, 3, 208, 4, 1}, maxValue, true, true});
    dataPoints.push_back({{157, 2, 235, 3, 1}, maxValue, true, true});
    dataPoints.push_back({{158, 3, 211, 4, 1}, maxValue, true, true});
    dataPoints.push_back({{159, 4, 199, 5, 1}, maxValue, true, true});
    dataPoints.push_back({{160, 140, 217, 190, minValue}, 0, true, true});
    dataPoints.push_back({{160, 4, 200, 5, 1}, maxValue, true, true});
    dataPoints.push_back({{161, 141, 217, 190, minValue}, 0, true, true});
    dataPoints.push_back({{161, 3, 215, 4, 1}, maxValue, true, true});
    dataPoints.push_back({{162, 142, 217, 190, 1}, maxValue, true, true});
    dataPoints.push_back({{163, 143, 217, 190, minValue}, 0, true, true});
    dataPoints.push_back({{164, 144, 217, 190, minValue}, 0, true, true});
    dataPoints.push_back({{165, 148, 213, 191, 1}, maxValue, true, true});
    dataPoints.push_back({{166, 3, 221, 4, 1}, maxValue, true, true});
    dataPoints.push_back({{167, 148, 215, 190, 1}, maxValue, true, true});
    dataPoints.push_back({{171, 165, 197, 190, 1}, maxValue, true, true});
    dataPoints.push_back({{171, 3, 228, 4, 1}, maxValue, true, true});
    dataPoints.push_back({{173, 3, 230, 4, 1}, maxValue, true, true});
    dataPoints.push_back({{174, 131, 254, 191, 1}, maxValue, true, true});
    dataPoints.push_back({{174, 3, 232, 4, 1}, maxValue, true, true});
    dataPoints.push_back({{175, 155, 217, 192, 1}, maxValue, true, true});
    dataPoints.push_back({{178, 150, 226, 190, 1}, maxValue, true, true});
    dataPoints.push_back({{178, 3, 237, 4, 1}, maxValue, true, true});
    dataPoints.push_back({{178, 4, 222, 5, 1}, maxValue, true, true});
    dataPoints.push_back({{181, 4, 226, 5, 1}, maxValue, true, true});
    dataPoints.push_back({{182, 0, 147, 2, minValue}, 0, false, false});
    dataPoints.push_back({{185, 152, 233, 191, 1}, maxValue, true, true});
    dataPoints.push_back({{186, 146, 240, 188, 1}, maxValue, true, true});
    dataPoints.push_back({{186, 4, 232, 5, 1}, maxValue, true, true});
    dataPoints.push_back({{188, 4, 235, 5, 1}, maxValue, true, true});
    dataPoints.push_back({{189, 142, 253, 190, 1}, maxValue, true, true});
    dataPoints.push_back({{19, 2, 57, 3, minValue}, 0, true, true});
    dataPoints.push_back({{193, 158, 235, 192, 1}, maxValue, true, true});
    dataPoints.push_back({{193, 4, 241, 5, 1}, maxValue, true, true});
    dataPoints.push_back({{197, 153, 246, 191, 1}, maxValue, true, true});
    dataPoints.push_back({{198, 170, 221, 190, 1}, maxValue, true, true});
    dataPoints.push_back({{199, 0, 114, 2, minValue}, 0, false, false});
    dataPoints.push_back({{199, 158, 242, 192, 1}, maxValue, true, true});
    dataPoints.push_back({{199, 3, 57, 3, minValue}, 0, false, false});
    dataPoints.push_back({{201, 160, 241, 192, 1}, maxValue, true, true});
    dataPoints.push_back({{202, 1, 54, 2, minValue}, 0, false, false});
    dataPoints.push_back({{202, 154, 251, 191, 1}, maxValue, true, true});
    dataPoints.push_back({{203, 155, 249, 190, 1}, maxValue, true, true});
    dataPoints.push_back({{203, 162, 236, 188, 1}, maxValue, true, true});
    dataPoints.push_back({{205, 157, 246, 188, 1}, maxValue, true, true});
    dataPoints.push_back({{208, 172, 229, 189, 1}, maxValue, true, true});
    dataPoints.push_back({{210, 160, 247, 188, 1}, maxValue, true, true});
    dataPoints.push_back({{212, 167, 241, 190, 1}, maxValue, true, true});
    dataPoints.push_back({{212, 187, 218, 192, 1}, maxValue, true, true});
    dataPoints.push_back({{213, 178, 229, 191, 1}, maxValue, true, true});
    dataPoints.push_back({{215, 188, 217, 190, minValue}, 0, true, true});
    dataPoints.push_back({{216, 165, 250, 191, 1}, maxValue, true, true});
    dataPoints.push_back({{216, 189, 217, 190, 1}, maxValue, true, true});
    dataPoints.push_back({{218, 179, 231, 190, 1}, maxValue, true, true});
    dataPoints.push_back({{218, 180, 233, 192, 1}, maxValue, true, true});
    dataPoints.push_back({{218, 2, 57, 3, minValue}, 0, false, false});
    dataPoints.push_back({{220, 185, 227, 191, 1}, maxValue, true, true});
    dataPoints.push_back({{229, 0, 54, 2, minValue}, 0, false, false});
    dataPoints.push_back({{234, 190, 235, 191, 1}, maxValue, true, true});
    dataPoints.push_back({{237, 0, 57, 3, minValue}, 0, false, false});
    dataPoints.push_back({{238, 185, 242, 188, 1}, maxValue, true, true});
    dataPoints.push_back({{240, 181, 251, 189, 1}, maxValue, true, true});
    dataPoints.push_back({{246, 186, 251, 190, 1}, maxValue, true, true});
    dataPoints.push_back({{248, 184, 253, 188, 1}, maxValue, true, true});
    dataPoints.push_back({{25, 2, 50, 4, 1}, maxValue, true, true});
    dataPoints.push_back({{25, 2, 75, 6, 1}, maxValue, true, true});
    dataPoints.push_back({{26, 23, 220, 191, 1}, maxValue, true, true});
    dataPoints.push_back({{27, 1, 108, 4, 1}, maxValue, true, true});
    dataPoints.push_back({{27, 1, 135, 5, 1}, maxValue, true, true});
    dataPoints.push_back({{27, 1, 54, 2, 1}, maxValue, true, true});
    dataPoints.push_back({{27, 1, 81, 3, 1}, maxValue, true, true});
    dataPoints.push_back({{31, 1, 123, 4, 1}, maxValue, true, true});
    dataPoints.push_back({{34, 1, 103, 3, 1}, maxValue, true, true});
    dataPoints.push_back({{34, 1, 171, 5, 1}, maxValue, true, true});
    dataPoints.push_back({{34, 2, 103, 6, 1}, maxValue, true, true});
    dataPoints.push_back({{38, 1, 189, 5, 1}, maxValue, true, true});
    dataPoints.push_back({{38, 2, 57, 3, 1}, maxValue, true, true});
    dataPoints.push_back({{38, 2, 76, 4, 1}, maxValue, true, true});
    dataPoints.push_back({{38, 2, 95, 5, 1}, maxValue, true, true});
    dataPoints.push_back({{38, 3, 57, 3, 1}, maxValue, true, true});
    dataPoints.push_back({{38, 30, 241, 188, 1}, maxValue, true, true});
    dataPoints.push_back({{41, 1, 122, 3, 1}, maxValue, true, true});
    dataPoints.push_back({{43, 1, 171, 4, 1}, maxValue, true, true});
    dataPoints.push_back({{45, 25, 66, 37, minValue}, 0, true, true});
    dataPoints.push_back({{46, 26, 66, 37, minValue}, 0, true, true});
    dataPoints.push_back({{48, 1, 239, 5, 1}, maxValue, true, true});
    dataPoints.push_back({{48, 27, 66, 37, 0}, 1, true, true});
    dataPoints.push_back({{50, 28, 66, 37, minValue}, 0, true, true});
    dataPoints.push_back({{50, 4, 75, 6, 1}, maxValue, true, true});
    dataPoints.push_back({{51, 1, 205, 4, 1}, maxValue, true, true});
    dataPoints.push_back({{52, 29, 66, 37, minValue}, 0, true, true});
    dataPoints.push_back({{54, 2, 108, 4, 1}, maxValue, true, true});
    dataPoints.push_back({{54, 2, 135, 5, 1}, maxValue, true, true});
    dataPoints.push_back({{54, 2, 81, 3, 1}, maxValue, true, true});
    dataPoints.push_back({{57, 1, 114, 2, 1}, maxValue, true, true});
    dataPoints.push_back({{57, 1, 171, 3, 1}, maxValue, true, true});
    dataPoints.push_back({{57, 1, 172, 3, 1}, maxValue, true, true});
    dataPoints.push_back({{57, 1, 228, 4, 1}, maxValue, true, true});
    dataPoints.push_back({{57, 2, 114, 4, 1}, maxValue, true, true});
    dataPoints.push_back({{57, 3, 76, 4, 1}, maxValue, true, true});
    dataPoints.push_back({{57, 3, 95, 5, 1}, maxValue, true, true});
    dataPoints.push_back({{59, 3, 79, 4, 1}, maxValue, true, true});
    dataPoints.push_back({{62, 2, 154, 5, 1}, maxValue, true, true});
    dataPoints.push_back({{62, 3, 104, 5, 1}, maxValue, true, true});
    dataPoints.push_back({{62, 3, 82, 4, 1}, maxValue, true, true});
    dataPoints.push_back({{65, 55, 226, 190, 1}, maxValue, true, true});
    dataPoints.push_back({{66, 1, 197, 3, 1}, maxValue, true, true});
    dataPoints.push_back({{68, 2, 171, 5, 1}, maxValue, true, true});
    dataPoints.push_back({{74, 1, 147, 2, 1}, maxValue, true, true});
    dataPoints.push_back({{76, 4, 95, 5, 1}, maxValue, true, true});
    dataPoints.push_back({{77, 2, 193, 5, 1}, maxValue, true, true});
    dataPoints.push_back({{77, 3, 129, 5, 1}, maxValue, true, true});
    dataPoints.push_back({{78, 1, 233, 3, 1}, maxValue, true, true});
    dataPoints.push_back({{79, 2, 158, 4, 1}, maxValue, true, true});
    dataPoints.push_back({{81, 3, 108, 4, 1}, maxValue, true, true});
    dataPoints.push_back({{81, 3, 135, 5, 1}, maxValue, true, true});
    dataPoints.push_back({{82, 63, 246, 188, 1}, maxValue, true, true});
    dataPoints.push_back({{86, 1, 171, 2, 1}, maxValue, true, true});
    dataPoints.push_back({{86, 1, 172, 2, 1}, maxValue, true, true});
    dataPoints.push_back({{86, 2, 171, 4, 1}, maxValue, true, true});
    dataPoints.push_back({{86, 3, 114, 4, 1}, maxValue, true, true});
    dataPoints.push_back({{87, 1, 174, 2, 1}, maxValue, true, true});
    dataPoints.push_back({{94, 2, 141, 3, 1}, maxValue, true, true});
    dataPoints.push_back({{94, 2, 188, 4, 1}, maxValue, true, true});
    dataPoints.push_back({{94, 2, 235, 5, 1}, maxValue, true, true});
    dataPoints.push_back({{95, 84, 217, 191, 1}, maxValue, true, true});
    dataPoints.push_back({{98, 3, 163, 5, 1}, maxValue, true, true});
    dataPoints.push_back({{98, 97, 191, 189, 1}, maxValue, true, true});*/

    for (auto &dp : dataPoints) {
        if (dp.positive) {
            dp.slope.Setup(0, 0, dp.dp.width, dp.dp.height, dp.left);
        } else {
            dp.slope.Setup(dp.dp.width, 0, 0, dp.dp.height, dp.left);
        }
    }

    using clk = std::chrono::steady_clock;
    using namespace std::chrono_literals;

    Context ctx;

    auto pga = std::make_unique<GAFuncSearch>("E:/Development/_refs/NDS/Research/Antialiasing");
    auto &ga = *pga;
    ga.SetTemplateOps(templateOps);
    ga.SetFixedDataPoints(dataPoints);

    auto updateInterval = 1000ms;
    auto t = clk::now();
    auto ts = t;
    auto t0 = t;
    auto tsleep = t + updateInterval;
    ga.SetResetCallback([&] { ts = clk::now(); });

    auto hndConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo;
    GetConsoleCursorInfo(hndConsole, &cursorInfo);
    cursorInfo.bVisible = false;
    SetConsoleCursorInfo(hndConsole, &cursorInfo);

    COORD cursorPos = {.X = 0, .Y = 0};

    auto newLine = [&] {
        CONSOLE_SCREEN_BUFFER_INFO buf{};
        GetConsoleScreenBufferInfo(hndConsole, &buf);
        DWORD len = buf.dwSize.X - buf.dwCursorPosition.X + 1;
        DWORD _;
        FillConsoleOutputCharacter(hndConsole, ' ', len, buf.dwCursorPosition, &_);
        std::cout << "\n";
    };

    auto blankBuffer = [&] {
        CONSOLE_SCREEN_BUFFER_INFO buf{};
        GetConsoleScreenBufferInfo(hndConsole, &buf);
        DWORD len1 = buf.dwSize.X - buf.dwCursorPosition.X + 1;
        DWORD lenRest = buf.dwSize.X * (buf.dwSize.Y - buf.dwCursorPosition.Y + 1);
        DWORD _;
        FillConsoleOutputCharacter(hndConsole, ' ', len1 + lenRest, buf.dwCursorPosition, &_);
    };

    auto printChrom = [&](const GAFuncSearch::Chromosome &chrom, bool printDisabled) {
        for (auto &gene : chrom.genes) {
            if (printDisabled) {
                std::cout << ' ';
                if (gene.enabled) {
                    std::cout << gene.op.Str();
                } else {
                    std::cout << '-';
                }
            } else {
                if (gene.enabled) {
                    std::cout << ' ' << gene.op.Str();
                }
            }
        }
    };

    auto printBest = [&](bool showAllResults) {
        SetConsoleCursorInfo(hndConsole, &cursorInfo);
        SetConsoleCursorPosition(hndConsole, cursorPos);
        const auto &best = ga.BestChromosome();
        const auto resetCount = ga.ResetCount();
        auto duration = t - ts;
        auto totalDuration = t - t0;
        std::cout << "Generation " << ga.CurrGeneration();
        std::cout << "    Time: " << std::fixed << std::setprecision(3)
                  << (std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() / 1000.0) << " sec";
        std::cout << "    Speed: " << std::fixed << std::setprecision(2)
                  << (double)ga.CurrGeneration() * 1000000000.0 / duration.count() << " gens/sec";
        newLine();
        std::cout << "  Resets: " << resetCount;
        std::cout << "    Total time: " << std::fixed << std::setprecision(3)
                  << (std::chrono::duration_cast<std::chrono::milliseconds>(totalDuration).count() / 1000.0) << " sec";
        newLine();
        std::cout << "  Best chromosome: fitness=" << best.fitness << ", errors=" << best.numErrors
                  << ", stack size=" << best.stackSize << ", generation=" << best.generation;
        newLine();
        std::cout << "  Function:";
        printChrom(best, true);
        newLine();

        for (auto &dp : dataPoints) {
            ctx.slope = dp.slope;
            ctx.stack.clear();
            ctx.vars.Apply(dp.dp, dp.left);

            bool valid = true;
            for (auto &gene : best.genes) {
                if (!gene.enabled) {
                    continue;
                }
                if (!gene.op.Execute(ctx)) {
                    valid = false;
                    break;
                }
            }
            if (!valid || ctx.stack.empty()) {
                continue;
            }
            i32 result = ctx.stack.back();
            /*result &= 1023;
            if (!dataPoint.left) {
                result ^= 1023;
            }*/
            if (showAllResults || result < dp.dp.expectedOutput || result > dp.upperBound) {
                std::cout << "   "                                              //
                          << std::setw(3) << std::right << dp.dp.width          //
                          << "x" << std::setw(3) << std::left << dp.dp.height   //
                          << " @ "                                              //
                          << std::setw(3) << std::right << dp.dp.x              //
                          << "x" << std::setw(3) << std::left << dp.dp.y        //
                          << "  "                                               //
                          << (dp.slope.IsLeftEdge() ? 'L' : 'R')                //
                          << (dp.slope.IsPositive() ? 'P' : 'N')                //
                          << (dp.slope.IsXMajor() ? 'X' : 'Y')                  //
                          << "  "                                               //
                          << std::setw(10) << std::right << result << " "       //
                          << std::setw(4) << std::right << dp.dp.expectedOutput //
                          << ".." << std::setw(4) << std::left << dp.upperBound;
                newLine();
            }
        }

        if (resetCount > 0) {
            std::cout << "  Previous best chromosomes:";
            newLine();
            size_t index = 0;
            for (auto &chrom : ga.BestChromosomesHistory()) {
                std::cout << "    " << index << ": [fitness=" << chrom.fitness << ", errors=" << chrom.numErrors
                          << ", stack size=" << chrom.stackSize << "]";
                printChrom(chrom, false);
                newLine();
                ++index;
            }
        }
        blankBuffer();
        if (!showAllResults) {
            SetConsoleCursorPosition(hndConsole, cursorPos);
        }
    };

    for (;;) {
        std::this_thread::sleep_until(tsleep);
        // ga.NextGeneration();
        t = clk::now();
        if (t >= tsleep) {
            if (t >= tsleep + updateInterval) {
                tsleep = t + updateInterval;
            } else {
                tsleep += updateInterval;
            }
            printBest(false);
        }
        const auto &best = ga.BestChromosome();
        if (best.fitness == 0) {
            ga.Stop();
            break;
        }
    }

    printBest(true);

    cursorInfo.bVisible = true;
    SetConsoleCursorInfo(hndConsole, &cursorInfo);

    std::cout << "----- Finished! -----\n";
    std::cout << "Type \"OK\" (all caps) to exit\n";
    std::string input;
    while (std::cin >> input) {
        if (input == "OK") {
            break;
        }
    }

    return EXIT_SUCCESS;
}

// --------------------------------------------------------------------------------

int main6() {
    constexpr i32 kConstants[] = {
        0,
        1,
        2,
        3,
        4,
        Slope::kAARange,
        Slope::kAARange - 1,
        Slope::kAAFracBits,
        Slope::kAAFracBits + 5,
        Slope::kAAFracRange,
        Slope::kAAFracRange - 1,
        Slope::kFracBits,
        Slope::kFracBits >> 1,
        Slope::kOne,
        Slope::kOne - 1,
        Slope::kBias,
    };
    std::vector<Operation> templateOps;
    {
        for (i32 c : kConstants) {
            templateOps.push_back(Operation{.type = Operation::Type::Constant, .constVal = c});
        }
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::PushX});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::PushY});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::PushWidth});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::PushHeight});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::PushPositive});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::PushNegative});
        // templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::PushXMajor});
        // templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::PushYMajor});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::PushLeft});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::PushRight});

        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Add});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Subtract});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Multiply});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Divide});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Modulo});
        // templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Negate});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::LeftShift});
        // templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::ArithmeticRightShift});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::LogicRightShift});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::And});
        // templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Or});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Xor});
        // templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Not});

        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Dup});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Over});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Swap});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Drop});
        // templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Rot});
        // templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::RevRot});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::IfElse});

        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::FracXStart});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::FracXEnd});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::FracXWidth});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::XStart});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::XEnd});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::XWidth});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::X0});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::InsertAAFracBits});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::MulWidth});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::MulHeight});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::DivWidth});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::DivHeight});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Add1});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Sub1});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Mul2});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Div2});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::MulHeightDivWidthAA});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::AAStep});
    }

    auto formula = generateFuncDFS(templateOps, "E:/Development/_refs/NDS/Research/Antialiasing");
    if (formula.empty()) {
        printf("No valid formula found\n");
    } else {
        printf("Found valid formula:");
        for (auto &op : formula) {
            auto opStr = op.Str();
            printf(" %s", opStr.c_str());
        }
        printf("\n");
    }

    return EXIT_SUCCESS;
}

// --------------------------------------------------------------------------------

int main7() {
    /*std::cout << "Extracting X-major bias data sets...\n";
    extractXMajorBiasDataSet("E:/Development/_refs/NDS/Research/Antialiasing");
    std::cout << "... done\n";*/

    std::cout << "Loading X-major bias data sets...";
    auto dataset = loadXMajorBiasDataSet("E:/Development/_refs/NDS/Research/Antialiasing");
    std::cout << " OK\n";

    struct DataPoints {
        std::unordered_set<u32> keys;
        std::unordered_map<u32, XMBDataPoint> lpx;
        std::unordered_map<u32, XMBDataPoint> lnx;
        std::unordered_map<u32, XMBDataPoint> rpx;
        std::unordered_map<u32, XMBDataPoint> rnx;
    };

    auto makeKey = [](u32 x, u32 y) -> u32 { return x | (y << 9); };

    std::unordered_map<u32, DataPoints> datapoints;
    for (auto &dp : dataset.lpx) {
        auto &dps = datapoints[makeKey(dp.width, dp.height)];
        dps.keys.insert(dp.y);
        dps.lpx[dp.y] = dp;
    }
    for (auto &dp : dataset.lnx) {
        auto &dps = datapoints[makeKey(dp.width, dp.height)];
        dps.keys.insert(dp.y);
        dps.lnx[dp.y] = dp;
    }
    for (auto &dp : dataset.rpx) {
        auto &dps = datapoints[makeKey(dp.width, dp.height)];
        dps.keys.insert(dp.y);
        dps.rpx[dp.y] = dp;
    }
    for (auto &dp : dataset.rnx) {
        auto &dps = datapoints[makeKey(dp.width, dp.height)];
        dps.keys.insert(dp.y);
        dps.rnx[dp.y] = dp;
    }

    std::cout << "  WxH  Y coord         LPX         LNX         RPX         RNX      Differences\n";
    for (u32 h = 0; h <= 192; h++) {
        for (u32 w = 0; w < 256; w++) { // ignoring W=256 due to a few possible errors in the data set
            const auto key = makeKey(w, h);
            if (!datapoints.contains(key)) {
                continue;
            }
            auto &dps = datapoints.at(key);
            for (auto key : dps.keys) {
                std::cout << std::setw(3) << std::right << w << 'x' << std::setw(3) << std::left << h;
                std::cout << "  y=" << std::setw(3) << std::left << key;
                std::cout << " -> ";

                // Sanity check: ensure all data sets have matching data points
                const bool lpxPresent = dps.lpx.contains(key);
                const bool lnxPresent = dps.lnx.contains(key);
                const bool rpxPresent = dps.rpx.contains(key);
                const bool rnxPresent = dps.rnx.contains(key);
                const bool allPresent = lpxPresent && lnxPresent && rpxPresent && rnxPresent;
                if (!allPresent) {
                    std::cout << "missing";
                    if (!lpxPresent) {
                        std::cout << " LPX";
                    }
                    if (!lnxPresent) {
                        std::cout << " LNX";
                    }
                    if (!rpxPresent) {
                        std::cout << " RPX";
                    }
                    if (!rnxPresent) {
                        std::cout << " RNX";
                    }
                    std::cout << '\n';
                    continue;
                }

                // Display bias ranges and check for discrepancies
                auto printBiasRange = [](const XMBDataPoint &dp) {
                    std::cout << "  ";
                    if (dp.biasLB >= 1024) {
                        // Shouldn't happen; every data point has a valid bias range
                        std::cout << "<unexpec.>";
                    } else {
                        std::cout << std::setw(4) << std::right << dp.biasLB;
                        if (dp.biasLB != dp.biasUB) {
                            std::cout << ".." << std::setw(4) << std::left << dp.biasUB;
                        } else {
                            std::cout << "      ";
                        }
                    }
                };

                auto &lpxDP = dps.lpx[key];
                auto &lnxDP = dps.lnx[key];
                auto &rpxDP = dps.rpx[key];
                auto &rnxDP = dps.rnx[key];

                printBiasRange(lpxDP);
                printBiasRange(lnxDP);
                printBiasRange(rpxDP);
                printBiasRange(rnxDP);
                std::cout << "  ";
                if (lpxDP.biasLB == lnxDP.biasLB && lpxDP.biasUB == lnxDP.biasUB) {
                    std::cout << "    ";
                } else {
                    std::cout << "  LL";
                }
                if (rpxDP.biasLB == rnxDP.biasLB && rpxDP.biasUB == rnxDP.biasUB) {
                    std::cout << "    ";
                } else {
                    std::cout << "  RR";
                }
                if (lpxDP.biasLB == rpxDP.biasLB && lpxDP.biasUB == rpxDP.biasUB) {
                    std::cout << "    ";
                } else {
                    std::cout << "  PP";
                }
                if (lnxDP.biasLB == rnxDP.biasLB && lnxDP.biasUB == rnxDP.biasUB) {
                    std::cout << "    ";
                } else {
                    std::cout << "  NN";
                }
                if (lpxDP.biasLB == rnxDP.biasLB && lpxDP.biasUB == rnxDP.biasUB) {
                    std::cout << "      ";
                } else {
                    std::cout << "  L+R-";
                }
                if (rpxDP.biasLB == lnxDP.biasLB && rpxDP.biasUB == lnxDP.biasUB) {
                    std::cout << "      ";
                } else {
                    std::cout << "  L-R+";
                }
                std::cout << '\n';

                // Observations:
                // - LPX+RPX and LNX+RNX match perfectly
                //   (i.e. PP and NN never happen)
                // - When positives and negatives mismatch, all _PX+_NX pairs mismatch together
                //   (i.e. LL, RR, L+R- and L-R+ always show up simultaneously)
            }
        }
    }

    return EXIT_SUCCESS;
}
