#include <bitset>
#include <chrono>
#include <iostream>
#include <sstream>

#include "dataset.h"
#include "file.h"
#include "func_generator.h"
#include "interactive_eval.h"
#include "tester.h"

int main1() {
    // convertScreenCap("data/screencap.bin", "data/screencap.tga");
    // uniqueColors("data/screencap.bin");

    // auto dataTL = readFile("data/TL.bin");
    // auto dataTR = readFile("data/TR.bin");
    // auto dataBL = readFile("data/BL.bin");
    // auto dataBR = readFile("data/BR.bin");

    // auto dataT = readFile("C:/temp/T.bin");
    // auto dataB = readFile("C:/temp/B.bin");
    // auto dataL = readFile("C:/temp/L.bin");
    auto dataR = readFile("C:/temp/R.bin");

    // if (dataT)
    //    test(*dataT);
    // if (dataB)
    //    test(*dataB);
    // if (dataL)
    //    test(*dataL);
    if (dataR)
        test(*dataR);

    // if (dataT) writeImages(*dataT, "C:/temp/T");
    // if (dataB) writeImages(*dataB, "C:/temp/B");
    // if (dataL) writeImages(*dataL, "C:/temp/L");
    // if (dataR) writeImages(*dataR, "C:/temp/R");

    return EXIT_SUCCESS;
}

// --------------------------------------------------------------------------------

int main2() {
    extractDataSet("C:/temp");
    return EXIT_SUCCESS;
}

// --------------------------------------------------------------------------------

int main() {
    runInteractiveEvaluator("C:/temp");
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
        templateOps.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::ExpandAABits});
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
