#pragma once

#include <iostream>
#include <ostream>
#include <string>
#include <vector>

#include "dataset.h"
#include "slope.h"
#include "types.h"

enum class Operator {
    // Variables

    PushX,        // pushes X
    PushY,        // pushes Y
    PushWidth,    // pushes width
    PushHeight,   // pushes height
    PushPositive, // pushes 1 if positive, 0 if negative
    PushNegative, // pushes 0 if positive, 1 if negative
    PushXMajor,   // pushes 1 if X-major, 0 if Y-major
    PushYMajor,   // pushes 0 if X-major, 1 if Y-major
    PushLeft,     // pushes 1 if left edge, 0 if right edge
    PushRight,    // pushes 0 if left edge, 1 if right edge

    // General operators

    Add,
    Subtract,
    Multiply,
    Divide,
    Modulo,
    Negate,
    LeftShift,
    ArithmeticRightShift, // signed right shift
    LogicRightShift,      // unsigned right shift
    And,
    Or,
    Xor,
    Not,

    // Stack operators

    Dup,    // duplicates the top item
    Swap,   // swaps the two top items
    Drop,   // discards the top item
    Rot,    // moves the top item to the Nth position, shifting items up
    RevRot, // takes the Nth from the stack and moves it to the top, shifting items down

    // Domain-specific operators

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

};

constexpr Operator kOperators[] = {
    Operator::PushX,
    Operator::PushY,
    Operator::PushWidth,
    Operator::PushHeight,
    Operator::PushPositive,
    Operator::PushNegative,
    Operator::PushXMajor,
    Operator::PushYMajor,
    Operator::PushLeft,
    Operator::PushRight,

    Operator::Add,
    Operator::Subtract,
    Operator::Multiply,
    Operator::Divide,
    Operator::Modulo,
    Operator::Negate,
    Operator::LeftShift,
    Operator::ArithmeticRightShift,
    Operator::LogicRightShift,
    Operator::And,
    Operator::Or,
    Operator::Xor,
    Operator::Not,

    Operator::Dup,
    Operator::Swap,
    Operator::Drop,
    Operator::Rot,
    Operator::RevRot,

    Operator::FracXStart,
    Operator::FracXEnd,
    Operator::FracXWidth,
    Operator::XStart,
    Operator::XEnd,
    Operator::XWidth,
    Operator::InsertAAFracBits,
    Operator::MulWidth,
    Operator::MulHeight,
    Operator::DivWidth,
    Operator::DivHeight,
    Operator::Div2,
    Operator::MulHeightDivWidthAA,

};

inline std::string OperatorName(Operator op) {
    switch (op) {
    case Operator::PushX: return "push_x";
    case Operator::PushY: return "push_y";
    case Operator::PushWidth: return "push_width";
    case Operator::PushHeight: return "push_height";
    case Operator::PushPositive: return "push_pos";
    case Operator::PushNegative: return "push_neg";
    case Operator::PushXMajor: return "push_xmaj";
    case Operator::PushYMajor: return "push_ymaj";
    case Operator::PushLeft: return "push_left";
    case Operator::PushRight: return "push_right";

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
    case Operator::RevRot: return "rrot";

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
    }
    return "(invalid op)";
}

struct Variables {
    i32 x, y;
    i32 width, height;
    bool left;

    void Apply(const DataPoint &dataPoint, bool left) {
        x = dataPoint.x;
        y = dataPoint.y;
        width = dataPoint.width;
        height = dataPoint.height;
        this->left = left;
    }
};

struct Context {
    Variables vars;
    std::vector<i32> stack;
    Slope slope;

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
        case Type::Operator: return OperatorName(op);
        case Type::Constant:
            return std::string("push_") + std::to_string(constVal);
            // return std::format("push_const {}", constVal);
        default: return "(invalid type)";
        }
    }

    /*[[gnu::flatten]]*/ bool Execute(Context &ctx) {
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
        case Operator::PushX: ctx.stack.push_back(ctx.vars.x); return true;
        case Operator::PushY: ctx.stack.push_back(ctx.vars.y); return true;
        case Operator::PushWidth: ctx.stack.push_back(ctx.vars.width); return true;
        case Operator::PushHeight: ctx.stack.push_back(ctx.vars.height); return true;
        case Operator::PushPositive: ctx.stack.push_back(!ctx.slope.IsNegative()); return true;
        case Operator::PushNegative: ctx.stack.push_back(ctx.slope.IsNegative()); return true;
        case Operator::PushXMajor: ctx.stack.push_back(ctx.slope.IsXMajor()); return true;
        case Operator::PushYMajor: ctx.stack.push_back(!ctx.slope.IsXMajor()); return true;
        case Operator::PushLeft: ctx.stack.push_back(ctx.vars.left); return true;
        case Operator::PushRight: ctx.stack.push_back(!ctx.vars.left); return true;

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
                std::rotate(stack.rbegin(), stack.rbegin() + 1, stack.rbegin() + count);
                return true;
            }

        case Operator::RevRot:
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
                std::rotate(stack.rbegin(), stack.rbegin() + count - 1, stack.rbegin() + count);
                return true;
            }

        case Operator::FracXStart: stack.push_back(ctx.slope.FracXStart(ctx.vars.y)); return true;
        case Operator::FracXEnd: stack.push_back(ctx.slope.FracXEnd(ctx.vars.y)); return true;
        case Operator::FracXWidth: stack.push_back(ctx.slope.DX()); return true;

        case Operator::XStart: stack.push_back(ctx.slope.XStart(ctx.vars.y)); return true;
        case Operator::XEnd: stack.push_back(ctx.slope.XEnd(ctx.vars.y)); return true;
        case Operator::XWidth:
            stack.push_back(ctx.slope.XEnd(ctx.vars.y) - ctx.slope.XStart(ctx.vars.y) + 1);
            return true;

        case Operator::InsertAAFracBits: return unaryFunc([](i32 x) { return (x * 32) << Slope::kAAFracBits; });
        case Operator::MulWidth: return unaryFunc([&](i32 x) { return x * ctx.vars.width; });
        case Operator::MulHeight: return unaryFunc([&](i32 x) { return x * ctx.vars.height; });
        case Operator::DivWidth: return unaryFunc([&](i32 x) { return x / ctx.vars.width; });
        case Operator::DivHeight: return unaryFunc([&](i32 x) { return x / ctx.vars.height; });
        case Operator::Div2: return unaryFunc([&](i32 x) { return x >> 1; });
        case Operator::MulHeightDivWidthAA:
            return unaryFunc(
                [&](i32 x) { return ((x * ctx.vars.height * 32) << Slope::kAAFracBits) / ctx.vars.width; });
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

    void BeginEval(const DataPoint &dataPoint, bool positive, bool left) {
        if (positive) {
            ctx.slope.Setup(0, 0, dataPoint.width, dataPoint.height);
        } else {
            ctx.slope.Setup(dataPoint.width, 0, 0, dataPoint.height);
        }
        ctx.stack.clear();
        ctx.vars.Apply(dataPoint, left);
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
        const i32 fullCoverage = ((deltaX * ctx.vars.height * Slope::kAARange) << Slope::kAAFracBits) / ctx.vars.width;
        const i32 coverageStep = fullCoverage / deltaX;
        const i32 coverageBias = coverageStep / 2;
        const i32 offset = ctx.vars.x - startX;
        const i32 fracCoverage = baseCoverage + offset * coverageStep;
        const i32 finalCoverage = (fracCoverage + coverageBias) % Slope::kAABase;
        result = finalCoverage >> Slope::kAAFracBits;
        return true;
    }

    bool Eval(const DataPoint &dataPoint, bool positive, bool left, i32 &result) {
        if (!EvalCommon(dataPoint, positive, left)) {
            return false;
        }

        result = ctx.stack.back();
        return true;
    }

    bool EvalXMajor(const DataPoint &dataPoint, bool positive, bool left, i32 &result) {
        if (!EvalCommon(dataPoint, positive, left)) {
            return false;
        }

        const i32 divResult = (Slope::kOne / ctx.vars.height);
        const i32 dx = ctx.vars.y * divResult * ctx.vars.width;
        const i32 fracStart = Slope::kBias + dx;
        const i32 startX = fracStart >> Slope::kFracBits;
        const i32 endX = ((fracStart & Slope::kMask) + dx - Slope::kOne) >> Slope::kFracBits;
        const i32 deltaX = endX - startX + 1;
        const i32 baseCoverage = ctx.stack.back();
        const i32 fullCoverage = ((deltaX * ctx.vars.height * Slope::kAARange) << Slope::kAAFracBits) / ctx.vars.width;
        const i32 coverageStep = fullCoverage / deltaX;
        const i32 coverageBias = coverageStep / 2;
        const i32 offset = ctx.vars.x - startX;
        const i32 fracCoverage = baseCoverage + offset * coverageStep;
        const i32 finalCoverage = (fracCoverage + coverageBias) % Slope::kAABase;
        result = finalCoverage >> Slope::kAAFracBits;
        return true;
    }

private:
    bool EvalCommon(const DataPoint &dataPoint, bool positive, bool left) {
        BeginEval(dataPoint, positive, left);
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
