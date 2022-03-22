#include <bitset>
#include <chrono>
#include <iostream>
#include <sstream>

#include "dataset.h"
#include "file.h"
#include "func_generator.h"
#include "func_search.h"
#include "interactive_eval.h"
#include "tester.h"

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
        // templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::PushPositive});
        // templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::PushNegative});
        // templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::PushXMajor});
        // templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::PushYMajor});
        // templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::PushLeft});
        // templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::PushRight});

        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Add});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Subtract});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Multiply});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Divide});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Modulo});
        // templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Negate});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::LeftShift});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::ArithmeticRightShift});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::LogicRightShift});
        // templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::And});
        // templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Or});
        // templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Xor});
        // templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Not});

        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Dup});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Swap});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Drop});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::Rot});
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::RevRot});

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

    dataPoints.push_back({{0, 0, 205, 2, 4}, true, true});
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
    dataPoints.push_back({{0, 6, 217, 7, 16}, true, true});

    using clk = std::chrono::steady_clock;
    using namespace std::chrono_literals;

    GAFuncSearch ga{"E:/Development/_refs/NDS/Research/Antialiasing"};
    ga.SetTemplateOps(templateOps);
    ga.SetFixedDataPoints(dataPoints);
    auto &pop = ga.Population();
    auto t = clk::now();
    auto ts = t;
    for (;;) {
        ga.NextGeneration();
        auto t2 = clk::now();
        if (t2 - t >= 1s) {
            t = t2;
            const auto &best = std::min_element(pop.begin(), pop.end());
            std::cout << "Generation " << ga.CurrGeneration() << ":\n";
            std::cout << "  Speed: " << std::fixed << std::setprecision(2)
                      << (double)ga.CurrGeneration() * 1000000000.0 /
                             std::chrono::duration_cast<std::chrono::nanoseconds>(t2 - ts).count()
                      << " gens/sec\n";
            std::cout << "  Best fitness: " << best->fitness << "\n";
            std::cout << "  Function:";
            for (auto &gene : best->genes) {
                std::cout << ' ';
                if (gene.enabled) {
                    std::cout << gene.op.Str();
                } else {
                    std::cout << '-';
                }
            }
            std::cout << "\n";
        }
    }

    return EXIT_SUCCESS;
}
