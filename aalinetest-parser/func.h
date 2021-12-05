#pragma once

#include <iostream>
#include <ostream>
#include <string>
#include <vector>

#include "dataset.h"
#include "slope.h"
#include "types.h"

enum class Operator {
    FracXStart,
    FracXEnd,
    FracXWidth,
    XStart,
    XEnd,
    XWidth,
    InsertAAFracBits,
    MulWidth,
    MulHeight,
    DivWidth,
    DivHeight,
    Div2,
    MulHeightDivWidthAA,

    PushX,
    PushY,
    PushWidth,
    PushHeight,
    PushPosNeg,    // pushes 1 if positive, 0 if negative
    PushXYMajor,   // pushes 1 if X-major, 0 if Y-major
    PushLeftRight, // pushed 1 if left edge, 0 if right edge

    Add,
    Subtract,
    Multiply,
    Divide,
    Modulo,
    Negate,
    LeftShift,
    ArithmeticRightShift,
    LogicRightShift,
    And,
    Or,
    Xor,
    Not,

    Dup,
    Swap,
    Drop,
    Rot,
};

struct Variables {
    i32 x, y;
    i32 width, height;
    bool positive;
    bool xMajor;
    bool left;

    void Apply(const DataPoint &dataPoint, bool positive, bool xMajor, bool left) {
        x = dataPoint.x;
        y = dataPoint.y;
        width = dataPoint.width;
        height = dataPoint.height;
        this->positive = positive;
        this->xMajor = xMajor;
        this->left = left;
    }
};

struct Context {
    Variables vars;
    std::vector<i32> stack;

private:
    friend struct Operation;
    friend struct Evaluator;
};

struct Operation {
    enum class Type { Operator, Constant };

    Type type;
    union {
        Operator op;
        i32 constVal;
    };

    std::string Str() const {
        switch (type) {
        case Type::Operator:
            switch (op) {
            case Operator::FracXStart: return "push_frac_x_start";
            case Operator::FracXEnd: return "push_frac_x_end";
            case Operator::FracXWidth: return "push_frac_x_width";
            case Operator::XStart: return "push_x_start";
            case Operator::XEnd: return "push_x_end";
            case Operator::XWidth: return "push_x_width";
            case Operator::InsertAAFracBits: return "insert_aa_frac_bits";
            case Operator::MulWidth: return "mul_width";
            case Operator::MulHeight: return "mul_height";
            case Operator::DivWidth: return "div_width";
            case Operator::DivHeight: return "div_height";
            case Operator::Div2: return "div_2";
            case Operator::MulHeightDivWidthAA: return "mul_height_div_width_aa";
            case Operator::PushX: return "push_x";
            case Operator::PushY: return "push_y";
            case Operator::PushWidth: return "push_width";
            case Operator::PushHeight: return "push_height";
            case Operator::PushPosNeg: return "push_pos_neg";
            case Operator::PushXYMajor: return "push_xy_major";
            case Operator::PushLeftRight: return "push_left_right";

            case Operator::Add: return "add";
            case Operator::Subtract: return "sub";
            case Operator::Multiply: return "mul";
            case Operator::Divide: return "div";
            case Operator::Modulo: return "mod";
            case Operator::Negate: return "neg";
            case Operator::LeftShift: return "shl";
            case Operator::ArithmeticRightShift: return "sar";
            case Operator::LogicRightShift: return "shr";
            case Operator::And: return "and";
            case Operator::Or: return "or";
            case Operator::Xor: return "xor";
            case Operator::Not: return "not";
            case Operator::Dup: return "dup";
            case Operator::Swap: return "swap";
            case Operator::Drop: return "drop";
            case Operator::Rot: return "rot";
            }
            return "(invalid op)";
        case Type::Constant:
            return std::string("push_") + std::to_string(constVal);
            // return std::format("push_const {}", constVal);
        default: return "(invalid type)";
        }
    }

    [[gnu::flatten]] bool Execute(Context &ctx) {
        switch (type) {
        case Type::Operator: return ExecuteOperator(ctx);
        case Type::Constant: return ExecuteConstant(ctx);
        }
        return false;
    }

private:
    bool ExecuteOperator(Context &ctx) {
        auto &stack = ctx.stack;
        auto binaryFunc = [&](auto &&func) -> bool {
            if (stack.size() < 2) {
                return false;
            }
            const i32 y = stack.back();
            stack.pop_back();
            const i32 x = stack.back();
            stack.pop_back();
            const i32 result = func(x, y);
            stack.push_back(result);
            return true;
        };

        auto unaryFunc = [&](auto &&func) -> bool {
            if (stack.size() < 1) {
                return false;
            }
            const i32 x = stack.back();
            stack.pop_back();
            const i32 result = func(x);
            stack.push_back(result);
            return true;
        };

        switch (op) {
        case Operator::FracXStart: {
            i32 divResult = (Slope::kOne / ctx.vars.height);
            i32 dx = ctx.vars.y * divResult * ctx.vars.width;
            stack.push_back(Slope::kBias + dx);
            return true;
        }
        case Operator::FracXEnd: {
            i32 divResult = (Slope::kOne / ctx.vars.height);
            i32 dx = ctx.vars.y * divResult * ctx.vars.width;
            i32 fracStart = Slope::kBias + dx;
            stack.push_back((fracStart & Slope::kMask) + dx - Slope::kOne);
            return true;
        }
        case Operator::FracXWidth: {
            i32 divResult = (Slope::kOne / ctx.vars.height);
            i32 dx = ctx.vars.y * divResult * ctx.vars.width;
            stack.push_back(dx);
            return true;
        }

        case Operator::XStart: {
            i32 divResult = (Slope::kOne / ctx.vars.height);
            i32 dx = ctx.vars.y * divResult * ctx.vars.width;
            stack.push_back((Slope::kBias + dx) >> Slope::kFracBits);
            return true;
        }
        case Operator::XEnd: {
            i32 divResult = (Slope::kOne / ctx.vars.height);
            i32 dx = ctx.vars.y * divResult * ctx.vars.width;
            i32 fracStart = Slope::kBias + dx;
            stack.push_back(((fracStart & Slope::kMask) + dx - Slope::kOne) >> Slope::kFracBits);
            return true;
        }

        case Operator::XWidth: {
            i32 divResult = (Slope::kOne / ctx.vars.height);
            i32 dx = ctx.vars.y * divResult * ctx.vars.width;
            i32 fracStart = Slope::kBias + dx;
            i32 xStart = fracStart >> Slope::kFracBits;
            i32 xEnd = ((fracStart & Slope::kMask) + dx - Slope::kOne) >> Slope::kFracBits;
            stack.push_back(xEnd - xStart + 1);
            return true;
        }
        case Operator::InsertAAFracBits: return unaryFunc([](i32 x) { return (x * 32) << Slope::kAAFracBitsX; });
        case Operator::MulWidth: return unaryFunc([&](i32 x) { return x * ctx.vars.width; });
        case Operator::MulHeight: return unaryFunc([&](i32 x) { return x * ctx.vars.height; });
        case Operator::DivWidth: return unaryFunc([&](i32 x) { return x / ctx.vars.width; });
        case Operator::DivHeight: return unaryFunc([&](i32 x) { return x / ctx.vars.height; });
        case Operator::Div2: return unaryFunc([&](i32 x) { return x >> 1; });
        case Operator::MulHeightDivWidthAA:
            return unaryFunc(
                [&](i32 x) { return ((x * ctx.vars.height * 32) << Slope::kAAFracBitsX) / ctx.vars.width; });
        case Operator::PushX: ctx.stack.push_back(ctx.vars.x); return true;
        case Operator::PushY: ctx.stack.push_back(ctx.vars.y); return true;
        case Operator::PushWidth: ctx.stack.push_back(ctx.vars.width); return true;
        case Operator::PushHeight: ctx.stack.push_back(ctx.vars.height); return true;
        case Operator::PushPosNeg: ctx.stack.push_back(ctx.vars.positive); return true;
        case Operator::PushXYMajor: ctx.stack.push_back(ctx.vars.xMajor); return true;
        case Operator::PushLeftRight: ctx.stack.push_back(ctx.vars.left); return true;

        case Operator::Add: return binaryFunc([](i32 x, i32 y) { return x + y; });
        case Operator::Subtract: return binaryFunc([](i32 x, i32 y) { return x - y; });
        case Operator::Multiply: return binaryFunc([](i32 x, i32 y) { return x * y; });
        case Operator::Divide:
            return binaryFunc([](i32 x, i32 y) { return y == 0 || (x == 0x80000000 && y == -1) ? INT32_MAX : x / y; });
        case Operator::Modulo:
            return binaryFunc([](i32 x, i32 y) { return y == 0 || (x == 0x80000000 && y == -1) ? 0 : x % y; });
        case Operator::Negate: return unaryFunc([](i32 x) { return -x; });
        case Operator::LeftShift: return binaryFunc([](i32 x, i32 y) { return x << y; });
        case Operator::ArithmeticRightShift: return binaryFunc([](i32 x, i32 y) { return x >> y; });
        case Operator::LogicRightShift: return binaryFunc([](i32 x, i32 y) { return (u32)x >> (u32)y; });
        case Operator::And: return binaryFunc([](i32 x, i32 y) { return x & y; });
        case Operator::Or: return binaryFunc([](i32 x, i32 y) { return x | y; });
        case Operator::Xor: return binaryFunc([](i32 x, i32 y) { return x ^ y; });
        case Operator::Not: return unaryFunc([](i32 x) { return ~x; });
        case Operator::Dup:
            if (stack.empty()) {
                return false;
            } else {
                stack.push_back(stack.back());
                return true;
            }
        case Operator::Swap:
            if (stack.size() < 2) {
                return false;
            } else {
                std::swap(stack[stack.size() - 1], stack[stack.size() - 2]);
                return true;
            }
        case Operator::Drop:
            if (stack.size() < 1) {
                return false;
            } else {
                stack.pop_back();
                return true;
            }
        case Operator::Rot:
            if (stack.size() < 1) {
                return false;
            } else {
                i32 count = stack.back();
                if (count < 1) {
                    return false;
                }
                if (stack.size() < count + 1) {
                    return false;
                }
                stack.pop_back();
                std::rotate(stack.begin(), stack.begin() + count - 1, stack.begin() + count);
                return true;
            }
        }
        return false;
    }

    bool ExecuteConstant(Context &ctx) {
        ctx.stack.push_back(constVal);
        return true;
    }
};

struct Evaluator {
    Context ctx;
    std::vector<Operation> ops;

    void DebugPrint(std::ostream &os) {
        os << "Variables:\n";
        os << "  x=" << ctx.vars.x << "\n";
        os << "  y=" << ctx.vars.y << "\n";
        os << "  w=" << ctx.vars.width << "\n";
        os << "  h=" << ctx.vars.height << "\n";

        os << "Operations:\n";
        for (auto &op : ops) {
            os << "  " << op.Str() << "\n";
        }
    }

    void BeginEval(const DataPoint &dataPoint, bool positive, bool xMajor, bool left) {
        ctx.stack.clear();
        ctx.vars.Apply(dataPoint, positive, xMajor, left);
    }

    bool EvalOp(size_t index) {
        if (index >= ops.size()) {
            return false;
        }
        auto &op = ops[index];
        if (!op.Execute(ctx)) {
            return false;
        }
        return true;
    }

    bool Result(i32 &result) const {
        if (ctx.stack.empty()) {
            return false;
        }
        result = ctx.stack.back();
        return true;
    }

    bool ResultXMajor(i32 &result) const {
        if (ctx.stack.empty()) {
            return false;
        }
        const i32 divResult = (Slope::kOne / ctx.vars.height);
        const i32 dx = ctx.vars.y * divResult * ctx.vars.width;
        const i32 fracStart = Slope::kBias + dx;
        const i32 startX = fracStart >> Slope::kFracBits;
        const i32 endX = ((fracStart & Slope::kMask) + dx - Slope::kOne) >> Slope::kFracBits;
        const i32 deltaX = endX - startX + 1;
        const i32 baseCoverage = ctx.stack.back();
        const i32 fullCoverage = ((deltaX * ctx.vars.height * Slope::kAARange) << Slope::kAAFracBitsX) / ctx.vars.width;
        const i32 coverageStep = fullCoverage / deltaX;
        const i32 coverageBias = coverageStep / 2;
        const i32 offset = ctx.vars.x - startX;
        const i32 fracCoverage = baseCoverage + offset * coverageStep;
        const i32 finalCoverage = (fracCoverage + coverageBias) % Slope::kAABaseX;
        result = finalCoverage >> Slope::kAAFracBitsX;
        return true;
    }

    bool Eval(const DataPoint &dataPoint, bool positive, bool xMajor, bool left, i32 &result) {
        if (!EvalCommon(dataPoint, positive, xMajor, left)) {
            return false;
        }

        result = ctx.stack.back();
        return true;
    }

    bool EvalXMajor(const DataPoint &dataPoint, bool positive, bool xMajor, bool left, i32 &result) {
        if (!EvalCommon(dataPoint, positive, xMajor, left)) {
            return false;
        }

        const i32 divResult = (Slope::kOne / ctx.vars.height);
        const i32 dx = ctx.vars.y * divResult * ctx.vars.width;
        const i32 fracStart = Slope::kBias + dx;
        const i32 startX = fracStart >> Slope::kFracBits;
        const i32 endX = ((fracStart & Slope::kMask) + dx - Slope::kOne) >> Slope::kFracBits;
        const i32 deltaX = endX - startX + 1;
        const i32 baseCoverage = ctx.stack.back();
        const i32 fullCoverage = ((deltaX * ctx.vars.height * Slope::kAARange) << Slope::kAAFracBitsX) / ctx.vars.width;
        const i32 coverageStep = fullCoverage / deltaX;
        const i32 coverageBias = coverageStep / 2;
        const i32 offset = ctx.vars.x - startX;
        const i32 fracCoverage = baseCoverage + offset * coverageStep;
        const i32 finalCoverage = (fracCoverage + coverageBias) % Slope::kAABaseX;
        result = finalCoverage >> Slope::kAAFracBitsX;
        return true;
    }

private:
    bool EvalCommon(const DataPoint &dataPoint, bool positive, bool xMajor, bool left) {
        BeginEval(dataPoint, positive, xMajor, left);
        for (auto &op : ops) {
            if (!op.Execute(ctx)) {
                return false;
            }
        }
        /*if (ctx.stack.size() != 1) {
            return false;
        }*/
        // NOTE: relaxed rule -- allows garbage on the stack; top of the stack contains the result
        if (ctx.stack.empty()) {
            return false;
        }
        return true;
    }
};
