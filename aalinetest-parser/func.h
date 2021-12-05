#pragma once

#include <ostream>
#include <string>
#include <vector>

#include "dataset.h"
#include "slope.h"
#include "types.h"

enum class Operator {
    FracXStart,
    FracXEnd,
    XStart,
    XEnd,
    XWidth,
    ExpandAABits,
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
};

struct Variables {
    i32 x, y;
    i32 width, height;

    void Apply(const DataPoint &dataPoint) {
        x = dataPoint.x;
        y = dataPoint.y;
        width = dataPoint.width;
        height = dataPoint.height;
    }
};

struct Context {
    Variables vars;

private:
    std::vector<i32> stack;

    friend struct Operation;
    friend struct Evaluator;
};

struct Operation {
    enum class Type { Operator, Constant };

    Type type;
    union {
        Operator op;
        i32 constVal;
        i32 varId;
    };

    std::string Str() {
        switch (type) {
        case Type::Operator:
            switch (op) {
            case Operator::FracXStart: return "push_frac_x_start";
            case Operator::FracXEnd: return "push_frac_x_end";
            case Operator::XStart: return "push_x_start";
            case Operator::XEnd: return "push_x_end";
            case Operator::XWidth: return "push_x_width";
            case Operator::ExpandAABits: return "expand_aa_bits";
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
            }
            return "(invalid op)";
        case Type::Constant:
            return std::string("push_const ") + std::to_string(constVal);
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
            return (fracStart & Slope::kMask) + dx - Slope::kOne;
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
            return ((fracStart & Slope::kMask) + dx - Slope::kOne) >> Slope::kFracBits;
        }

        case Operator::XWidth: {
            i32 divResult = (Slope::kOne / ctx.vars.height);
            i32 dx = ctx.vars.y * divResult * ctx.vars.width;
            i32 fracStart = Slope::kBias + dx;
            i32 xStart = fracStart >> Slope::kFracBits;
            i32 xEnd = ((fracStart & Slope::kMask) + dx - Slope::kOne) >> Slope::kFracBits;
            return xEnd - xStart + 1;
        }
        case Operator::ExpandAABits: return unaryFunc([](i32 x) { return (x * 32) << Slope::kAAFracBitsX; });
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

    bool Eval(i32 &result) {
        ctx.stack.clear();
        for (auto &op : ops) {
            if (!op.Execute(ctx)) {
                return false;
            }
        }
        if (ctx.stack.size() != 1) {
            return false;
        }

        result = ctx.stack.back();

        return true;
    }

    bool EvalXMajor(i32 &result) {
        ctx.stack.clear();
        for (auto &op : ops) {
            if (!op.Execute(ctx)) {
                return false;
            }
        }
        if (ctx.stack.size() != 1) {
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
};
