#include "dataset.h"
#include "file.h"
#include "func_generator.h"
#include "func_search.h"
#include "interactive_eval.h"
#include "tester.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <bitset>
#include <chrono>
#include <iostream>
#include <memory>
#include <sstream>

int main1() {
    // convertScreenCap("data/screencap.bin", "data/screencap.tga");
    // uniqueColors("data/screencap.bin");

    auto dataT = readFile("E:/Development/_refs/NDS/Research/Antialiasing/T.bin");
    // auto dataB = readFile("E:/Development/_refs/NDS/Research/Antialiasing/B.bin");

    if (dataT)
        test(*dataT);
    // if (dataB)
    //     test(*dataB);

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

int main() {
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
        // templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Xor});
        // templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Not});

        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Dup});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Swap});
        // templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Drop});
        // templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Rot});
        // templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::RevRot});

        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::FracXStart});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::FracXEnd});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::FracXWidth});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::XStart});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::XEnd});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::XWidth});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::InsertAAFracBits});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::MulWidth});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::MulHeight});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::DivWidth});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::DivHeight});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Mul2});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Div2});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::MulHeightDivWidthAA});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::AAStep});
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

    // TODO: allow specifying lower and upper bound for the result
    // Bias
    dataPoints.push_back({{0, 0, 54, 2, 18}, 18, true, true});
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
    // dataPoints.push_back({{145, 11, 150, 12, 0}, 4, true, true});

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

    auto updateInterval = 250ms;
    auto t = clk::now();
    auto ts = t;
    auto tsleep = t + updateInterval;

    auto hndConsole = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO cursorInfo = {.dwSize = 1, .bVisible = false};
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
        std::cout << "\n";
    };

    auto printBest = [&](bool showAllResults) {
        SetConsoleCursorPosition(hndConsole, cursorPos);
        const auto &best = ga.BestChromosome();
        std::cout << "Generation " << ga.CurrGeneration() << ":";
        newLine();
        std::cout << "  Speed: " << std::fixed << std::setprecision(2)
                  << (double)ga.CurrGeneration() * 1000000000.0 /
                         std::chrono::duration_cast<std::chrono::nanoseconds>(t - ts).count()
                  << " gens/sec";
        newLine();
        std::cout << "  Best fitness: " << best.fitness;
        newLine();
        std::cout << "  Function:";
        for (auto &gene : best.genes) {
            std::cout << ' ';
            if (gene.enabled) {
                std::cout << gene.op.Str();
            } else {
                std::cout << '-';
            }
        }
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
                std::cout << "   " << dp.dp.width << "x" << dp.dp.height << " @ " << dp.dp.x << "x" << dp.dp.y << "  "
                          << result << " " << dp.dp.expectedOutput << ".." << dp.upperBound;
                newLine();
            }
        }
        blankBuffer();
    };

    for (;;) {
        std::this_thread::sleep_until(tsleep);
        // ga.NextGeneration();
        t = clk::now();
        if (t >= tsleep) {
            tsleep += updateInterval;
            printBest(false);
        }
        const auto &best = ga.BestChromosome();
        if (best.fitness == 0) {
            ga.Stop();
            break;
        }
    }

    std::cout << "----- Finished! -----\n";
    printBest(true);

    return EXIT_SUCCESS;
}
