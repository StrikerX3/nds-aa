#include "interactive_eval.h"

#include "dataset.h"
#include "func.h"

#include <algorithm>
#include <fstream>
#include <iostream>
#include <map>

std::string Uppercase(std::string str) {
    std::string out;
    std::transform(str.begin(), str.end(), std::back_inserter(out), [](auto c) { return std::toupper(c); });
    return out;
}

std::string Lowercase(std::string str) {
    std::string out;
    std::transform(str.begin(), str.end(), std::back_inserter(out), [](auto c) { return std::tolower(c); });
    return out;
}

enum class Group { LPX, LPY, LNX, LNY, RPX, RPY, RNX, RNY };

constexpr Group kGroups[] = {Group::LPX, Group::LPY, Group::LNX, Group::LNY,
                             Group::RPX, Group::RPY, Group::RNX, Group::RNY};

std::string GroupName(Group group) {
    switch (group) {
    case Group::LPX: return "LPX";
    case Group::LPY: return "LPY";
    case Group::LNX: return "LNX";
    case Group::LNY: return "LNY";
    case Group::RPX: return "RPX";
    case Group::RPY: return "RPY";
    case Group::RNX: return "RNX";
    case Group::RNY: return "RNY";
    default: return "invalid";
    }
}

bool GroupPositive(Group group) {
    switch (group) {
    case Group::LPX: return true;
    case Group::LPY: return true;
    case Group::LNX: return false;
    case Group::LNY: return false;
    case Group::RPX: return true;
    case Group::RPY: return true;
    case Group::RNX: return false;
    case Group::RNY: return false;
    default: return false;
    }
}

bool GroupXMajor(Group group) {
    switch (group) {
    case Group::LPX: return true;
    case Group::LPY: return false;
    case Group::LNX: return true;
    case Group::LNY: return false;
    case Group::RPX: return true;
    case Group::RPY: return false;
    case Group::RNX: return true;
    case Group::RNY: return false;
    default: return false;
    }
}

bool GroupLeft(Group group) {
    switch (group) {
    case Group::LPX: return true;
    case Group::LPY: return true;
    case Group::LNX: return true;
    case Group::LNY: return true;
    case Group::RPX: return false;
    case Group::RPY: return false;
    case Group::RNX: return false;
    case Group::RNY: return false;
    default: return false;
    }
}

bool GroupFromName(std::string name, Group &group) {
    name = Uppercase(name);
    if (name == "LPX") {
        group = Group::LPX;
        return true;
    } else if (name == "LPY") {
        group = Group::LPY;
        return true;
    } else if (name == "LNX") {
        group = Group::LNX;
        return true;
    } else if (name == "LNY") {
        group = Group::LNY;
        return true;
    } else if (name == "RPX") {
        group = Group::RPX;
        return true;
    } else if (name == "RPY") {
        group = Group::RPY;
        return true;
    } else if (name == "RNX") {
        group = Group::RNX;
        return true;
    } else if (name == "RNY") {
        group = Group::RNY;
        return true;
    } else {
        return false;
    }
}

class InteractiveEvaluator {
public:
    InteractiveEvaluator(DataSet &&dataset)
        : m_dataset(dataset) {}

    template <typename Func>
    bool Eval(Group group, Func &&func) {
        for (auto &dataPoint : GetDataSet(group)) {
            if (i32 result;
                m_eval.Eval(dataPoint, GroupPositive(group), GroupXMajor(group), GroupLeft(group), result)) {
                func(dataPoint, result);
            } else {
                return false;
            }
        }
        return true;
    }

    template <typename Func>
    bool Eval(Group group, i32 width, i32 height, Func &&func) {
        for (auto &dataPoint : GetDataSet(group)) {
            if (dataPoint.width == width && dataPoint.height == height) {
                if (i32 result;
                    m_eval.Eval(dataPoint, GroupPositive(group), GroupXMajor(group), GroupLeft(group), result)) {
                    func(dataPoint, result);
                } else {
                    return false;
                }
            }
        }
        return true;
    }

    std::vector<Operation> &Operations() {
        return m_eval.ops;
    }

    const std::vector<DataPoint> &GetDataSet(Group group) const {
        switch (group) {
        case Group::LPX: return m_dataset.lpx;
        case Group::LPY: return m_dataset.lpy;
        case Group::LNX: return m_dataset.lnx;
        case Group::LNY: return m_dataset.lny;
        case Group::RPX: return m_dataset.rpx;
        case Group::RPY: return m_dataset.rpy;
        case Group::RNX: return m_dataset.rnx;
        case Group::RNY: return m_dataset.rny;
        default: return m_dataset.lpx; // TODO: invalid/unreachable
        }
    }

    // --- Step evaluation -------------------------------------------------------------------------

    bool BeginStepEval(Group group, i32 width, i32 height, i32 x, i32 y) {
        for (auto &dataPoint : GetDataSet(group)) {
            if (dataPoint.x == x && dataPoint.y == y && dataPoint.width == width && dataPoint.height == height) {
                m_stepEvalActive = true;
                m_stepEvalIndex = 0;
                m_stepEvalGroup = group;
                m_stepEvalDataPoint = dataPoint;
                m_stepEval.ops = m_eval.ops;
                m_stepEval.BeginEval(dataPoint, GroupPositive(group), GroupXMajor(group), GroupLeft(group));
                return true;
            }
        }
        return false;
    }

    void StepEvalReset() {
        if (m_stepEvalActive) {
            m_stepEvalIndex = 0;
            m_stepEval.ops = m_eval.ops;
            m_stepEval.BeginEval(m_stepEvalDataPoint, GroupPositive(m_stepEvalGroup), GroupXMajor(m_stepEvalGroup),
                                 GroupLeft(m_stepEvalGroup));
        }
    }

    void StepEvalCancel() {
        m_stepEvalActive = false;
    }

    bool StepEval() {
        if (m_stepEval.EvalOp(m_stepEvalIndex)) {
            m_stepEvalIndex++;
            return true;
        } else {
            return false;
        }
    }

    bool IsStepping() const {
        return m_stepEvalActive;
    }

    size_t StepEvalIndex() const {
        return m_stepEvalIndex;
    }

    Group StepEvalGroup() const {
        return m_stepEvalGroup;
    }

    DataPoint StepEvalDataPoint() const {
        return m_stepEvalDataPoint;
    }

    const std::vector<i32> &StepEvalStack() const {
        return m_stepEval.ctx.stack;
    }

    const std::vector<Operation> &StepEvalOperations() const {
        return m_stepEval.ops;
    }

    bool StepEvalResult(i32 &result) const {
        return m_stepEval.Result(result);
    }

private:
    DataSet m_dataset;
    Evaluator m_eval;

    bool m_stepEvalActive = false;
    size_t m_stepEvalIndex = 0;
    Group m_stepEvalGroup;
    DataPoint m_stepEvalDataPoint;
    Evaluator m_stepEval;
};

void evaluate(InteractiveEvaluator &eval, Group group) {
    struct Output {
        DataPoint dataPoint;
        i32 result;
    };

    std::cout << GroupName(group) << ": ";

    std::vector<Output> mismatches;
    mismatches.reserve(10);
    u32 mismatchCount = 0;
    u32 totalError = 0;

    bool hasEntries = false;
    bool valid = eval.Eval(group, [&](const DataPoint &dataPoint, i32 result) {
        hasEntries = true;
        if (result != dataPoint.expectedOutput && mismatches.size() < 10) {
            mismatches.push_back(Output{dataPoint, result});
        }
        mismatchCount++;
        totalError += std::abs(result - dataPoint.expectedOutput);
    });

    if (valid) {
        if (!hasEntries) {
            std::cout << "No entries\n";
        } else if (totalError == 0) {
            std::cout << "Perfect match!\n";
        } else {
            std::cout << mismatchCount << " mismatch";
            if (mismatchCount != 1) {
                std::cout << "es";
            }
            std::cout << " with a total error of " << totalError << "\n";

            std::cout << "First " << mismatches.size() << " mismatches:\n";
            for (auto &mismatch : mismatches) {
                std::cout << "  " << mismatch.dataPoint.KeyStr() << "   " << mismatch.result
                          << " != " << mismatch.dataPoint.expectedOutput << "\n";
            }
        }
    } else {
        std::cout << "Invalid formula\n";
    }
}

void evaluate(InteractiveEvaluator &eval, Group group, i32 width, i32 height) {
    std::cout << GroupName(group) << " " << width << "x" << height << ":\n";

    u32 mismatchCount = 0;
    u32 totalError = 0;

    bool hasEntries = false;
    bool valid = eval.Eval(group, width, height, [&](const DataPoint &dataPoint, i32 result) {
        hasEntries = true;
        std::cout << "  " << dataPoint.KeyStr() << "  ->  " << result
                  << ((result == dataPoint.expectedOutput) ? " == " : " != ") << dataPoint.expectedOutput << "\n";
        mismatchCount++;
        totalError += std::abs(result - dataPoint.expectedOutput);
    });

    if (valid) {
        if (!hasEntries) {
            std::cout << "No entries\n";
        } else if (totalError == 0) {
            std::cout << "Perfect match!\n";
        } else {
            std::cout << mismatchCount << " mismatch";
            if (mismatchCount != 1) {
                std::cout << "es";
            }
            std::cout << " with a total error of " << totalError << "\n";
        }
    } else {
        std::cout << "Invalid formula\n";
    }
}

struct InteractiveContext {
    InteractiveEvaluator eval;
    bool running = true;
};

using FnCommand = void (*)(InteractiveContext &ctx, const std::vector<std::string> &args);

struct CommandDesc {
    std::vector<std::string> aliases;
    std::string description;

    std::string BuildAliases() {
        std::ostringstream os;
        std::string sep = "";
        for (auto &alias : aliases) {
            os << sep << alias;
            sep = ", ";
        }
        return os.str();
    }
};

std::map<std::string, FnCommand> commandFns;
std::vector<CommandDesc> commands;
size_t longestCommand;

namespace util {

void displayFunc(const std::vector<Operation> &ops, std::string marker = "", size_t markerStart = ~0,
                 size_t markerEnd = 0) {
    size_t i = 0;
    std::string spaces(marker.size(), ' ');
    for (auto &op : ops) {
        if (i >= markerStart && i < markerEnd) {
            std::cout << marker;
        } else {
            std::cout << spaces;
        }
        std::cout << i << ": " << op.Str() << "\n";
        i++;
    }
}

bool parseNum(const std::string &arg, i32 &output) {
    try {
        output = std::stoi(arg);
        return true;
    } catch (...) {
        std::cout << "Not a number: \"" << arg << "\"\n";
        return false;
    }
}

void displayStepEvalResult(InteractiveEvaluator &eval) {
    auto dataPoint = eval.StepEvalDataPoint();
    if (i32 result; eval.StepEvalResult(result)) {
        std::cout << "Result: " << result;
        if (result == dataPoint.expectedOutput) {
            std::cout << " == ";
        } else {
            std::cout << " != ";
        }
        std::cout << dataPoint.expectedOutput;
        std::cout << "\n";
    } else {
        std::cout << "Stack is empty; no result available\n";
    }
}

void displayStepEvalState(InteractiveEvaluator &eval) {
    auto dataPoint = eval.StepEvalDataPoint();
    std::cout << "Step-by-step evaluator state:\n";
    std::cout << "  Evaluating data point " << dataPoint.KeyStr() << " of group " << GroupName(eval.StepEvalGroup())
              << "\n";
    std::cout << "Operations:\n";
    util::displayFunc(eval.StepEvalOperations(), "=> ", eval.StepEvalIndex(), eval.StepEvalIndex() + 1);
    std::cout << "Stack:\n";
    size_t i = 0;
    for (auto num : eval.StepEvalStack()) {
        std::cout << "   " << i << ": " << num << "\n";
        i++;
    }
    if (i32 result; eval.StepEvalResult(result)) {
        std::cout << "Current  result: " << result << "\n";
        std::cout << "Expected result: " << dataPoint.expectedOutput << "\n";
    }
}

bool stepEval(InteractiveEvaluator &eval) {
    if (eval.StepEval()) {
        std::cout << eval.StepEvalOperations()[eval.StepEvalIndex() - 1].Str() << " =>";
        for (auto num : eval.StepEvalStack()) {
            std::cout << " " << num;
        }
        std::cout << "\n";
        return true;
    } else {
        std::cout << "Reached the end of the formula.\n";
        return false;
    }
}

} // namespace util

namespace command {

void quit(InteractiveContext &ctx, const std::vector<std::string> &) {
    ctx.running = false;
}

void help(InteractiveContext &, const std::vector<std::string> &) {
    std::cout << "Available commands:\n";
    for (auto &cmd : commands) {
        std::cout << "  " << std::setw(longestCommand) << cmd.BuildAliases() << "    ";
        std::string line;
        std::istringstream in{cmd.description};
        bool firstLine = true;
        while (std::getline(in, line)) {
            if (!firstLine) {
                std::cout << std::setw(longestCommand + 4 + 2) << " ";
            } else {
                firstLine = false;
            }
            std::cout << line << "\n";
        }
    }
}

void stepEval(InteractiveContext &ctx, const std::vector<std::string> &args) {
    using util::parseNum;

    if (args.size() < 5) {
        std::cout << "Missing arguments\n";
        std::cout << "Usage: eval group width height x y\n";
        std::cout << "  group: one of LPX, LPY, LNX, LNY, RPX, RPY, RNX, RNY\n";
        return;
    }

    if (Group group; GroupFromName(args[0], group)) {
        i32 width, height, x, y;
        if (!parseNum(args[1], width) || !parseNum(args[2], height) || !parseNum(args[3], x) || !parseNum(args[4], y)) {
            return;
        }
        if (!ctx.eval.BeginStepEval(group, width, height, x, y)) {
            std::cout << "No such data point found in group \"" << GroupName(group) << "\": " << width << "x" << height
                      << " @ " << x << "x" << y << ".\n";
            return;
        }
        std::cout << "Starting evaluation of data point " << width << "x" << height << " @ " << x << "x" << y
                  << " of group " << GroupName(group) << "\n";
    } else {
        std::cout << "Invalid group: \"" << GroupName(group) << "\"\n";
    }
}

void stepNext(InteractiveContext &ctx, const std::vector<std::string> &args) {
    using util::parseNum;

    if (!ctx.eval.IsStepping()) {
        std::cout << "Not running a step-by-step evaluation. Use the \"eval\" command to start.\n";
        return;
    }

    i32 count = 1;
    if (args.size() >= 1) {
        if (!parseNum(args[0], count)) {
            return;
        }
    }
    if (count < 1) {
        std::cout << "Invalid operation count; must be a positive number.\n";
        return;
    }

    for (i32 i = 0; i < count; i++) {
        if (!util::stepEval(ctx.eval)) {
            break;
        }
    }

    util::displayStepEvalResult(ctx.eval);
    util::displayStepEvalState(ctx.eval);
}

void stepGo(InteractiveContext &ctx, const std::vector<std::string> &args) {
    using util::parseNum;

    if (!ctx.eval.IsStepping()) {
        std::cout << "Not running a step-by-step evaluation. Use the \"eval\" command to start.\n";
        return;
    }

    for (;;) {
        if (!util::stepEval(ctx.eval)) {
            break;
        }
    }

    util::displayStepEvalResult(ctx.eval);
    util::displayStepEvalState(ctx.eval);
}

void stepReset(InteractiveContext &ctx, const std::vector<std::string> &args) {
    if (!ctx.eval.IsStepping()) {
        std::cout << "Not running a step-by-step evaluation. Use the \"eval\" command to start.\n";
        return;
    }
    ctx.eval.StepEvalReset();
    std::cout << "Step-by-step evaluation reset.\n";
}

void stepState(InteractiveContext &ctx, const std::vector<std::string> &args) {
    if (!ctx.eval.IsStepping()) {
        std::cout << "Not running a step-by-step evaluation. Use the \"eval\" command to start.\n";
        return;
    }
    util::displayStepEvalState(ctx.eval);
}

void stepCancel(InteractiveContext &ctx, const std::vector<std::string> &args) {
    if (!ctx.eval.IsStepping()) {
        std::cout << "Not running a step-by-step evaluation. Use the \"eval\" command to start.\n";
        return;
    }
    ctx.eval.StepEvalCancel();
    std::cout << "Step-by-step evaluation cancelled.\n";
}

void eval(InteractiveContext &ctx, const std::vector<std::string> &args) {
    using util::parseNum;

    if (args.empty()) {
        for (auto group : kGroups) {
            evaluate(ctx.eval, group);
        }
    } else {
        size_t argIndex = 0;
        while (argIndex < args.size()) {
            if (Group group; GroupFromName(args[argIndex], group)) {
                if (args.size() - argIndex >= 3) {
                    i32 width, height;
                    try {
                        width = std::stoi(args[argIndex + 1]);
                        height = std::stoi(args[argIndex + 2]);
                        evaluate(ctx.eval, group, width, height);
                        argIndex += 3;
                    } catch (...) {
                        evaluate(ctx.eval, group);
                        argIndex += 1;
                    }
                } else {
                    evaluate(ctx.eval, group);
                    argIndex += 1;
                }
            } else {
                std::cout << "Invalid group: \"" << args[argIndex] << "\"\n";
                argIndex += 1;
            }
        }
    }
}

void func(InteractiveContext &ctx, const std::vector<std::string> &) {
    util::displayFunc(ctx.eval.Operations(), "   ");
}

auto opTemplates = [] {
    std::map<std::string, Operation> templates;
    templates.insert({"add", {.type = Operation::Type::Operator, .op = Operator::Add}});
    templates.insert({"sub", {.type = Operation::Type::Operator, .op = Operator::Subtract}});
    templates.insert({"mul", {.type = Operation::Type::Operator, .op = Operator::Multiply}});
    templates.insert({"div", {.type = Operation::Type::Operator, .op = Operator::Divide}});
    templates.insert({"mod", {.type = Operation::Type::Operator, .op = Operator::Modulo}});
    templates.insert({"neg", {.type = Operation::Type::Operator, .op = Operator::Negate}});
    templates.insert({"shl", {.type = Operation::Type::Operator, .op = Operator::LeftShift}});
    templates.insert({"sar", {.type = Operation::Type::Operator, .op = Operator::ArithmeticRightShift}});
    templates.insert({"shr", {.type = Operation::Type::Operator, .op = Operator::LogicRightShift}});
    templates.insert({"and", {.type = Operation::Type::Operator, .op = Operator::And}});
    templates.insert({"or", {.type = Operation::Type::Operator, .op = Operator::Or}});
    templates.insert({"xor", {.type = Operation::Type::Operator, .op = Operator::Xor}});
    templates.insert({"not", {.type = Operation::Type::Operator, .op = Operator::Not}});

    templates.insert({"dup", {.type = Operation::Type::Operator, .op = Operator::Dup}});
    templates.insert({"swap", {.type = Operation::Type::Operator, .op = Operator::Swap}});
    templates.insert({"drop", {.type = Operation::Type::Operator, .op = Operator::Drop}});
    templates.insert({"rot", {.type = Operation::Type::Operator, .op = Operator::Rot}});

    templates.insert({"push_frac_x_start", {.type = Operation::Type::Operator, .op = Operator::FracXStart}});
    templates.insert({"push_frac_x_end", {.type = Operation::Type::Operator, .op = Operator::FracXEnd}});
    templates.insert({"push_frac_x_width", {.type = Operation::Type::Operator, .op = Operator::FracXWidth}});
    templates.insert({"push_x_start", {.type = Operation::Type::Operator, .op = Operator::XStart}});
    templates.insert({"push_x_end", {.type = Operation::Type::Operator, .op = Operator::XEnd}});
    templates.insert({"push_x_width", {.type = Operation::Type::Operator, .op = Operator::XWidth}});
    templates.insert({"insert_aa_frac_bits", {.type = Operation::Type::Operator, .op = Operator::InsertAAFracBits}});
    templates.insert({"mul_width", {.type = Operation::Type::Operator, .op = Operator::MulWidth}});
    templates.insert({"mul_height", {.type = Operation::Type::Operator, .op = Operator::MulHeight}});
    templates.insert({"div_width", {.type = Operation::Type::Operator, .op = Operator::DivWidth}});
    templates.insert({"div_height", {.type = Operation::Type::Operator, .op = Operator::DivHeight}});
    templates.insert({"div_2", {.type = Operation::Type::Operator, .op = Operator::Div2}});
    templates.insert(
        {"mul_height_div_width_aa", {.type = Operation::Type::Operator, .op = Operator::MulHeightDivWidthAA}});

    templates.insert({"push_x", {.type = Operation::Type::Operator, .op = Operator::PushX}});
    templates.insert({"push_y", {.type = Operation::Type::Operator, .op = Operator::PushY}});
    templates.insert({"push_width", {.type = Operation::Type::Operator, .op = Operator::PushWidth}});
    templates.insert({"push_height", {.type = Operation::Type::Operator, .op = Operator::PushHeight}});
    templates.insert({"push_pos_neg", {.type = Operation::Type::Operator, .op = Operator::PushPosNeg}});
    templates.insert({"push_xy_major", {.type = Operation::Type::Operator, .op = Operator::PushXYMajor}});
    templates.insert({"push_left_right", {.type = Operation::Type::Operator, .op = Operator::PushLeftRight}});

    return templates;
}();

void parseAndAddOp(std::vector<Operation> &ops, std::string op) {
    if (opTemplates.contains(op)) {
        ops.push_back(opTemplates.at(op));
    } else if (op.starts_with("push_")) {
        auto num = op.substr(5);
        try {
            Operation operation;
            operation.constVal = std::stoi(num);
            operation.type = Operation::Type::Constant;
            ops.push_back(operation);
        } catch (...) {
            std::cout << "Invalid constant: \"" << num << "\" -- ignored\n";
        }
    } else {
        std::cout << "Invalid operation: \"" << op << "\" -- ignored\n";
    }
}

void addOp(InteractiveContext &ctx, const std::vector<std::string> &args) {
    if (args.size() < 1) {
        std::cout << "Missing argument: op\n";
        std::cout << "Usage: addop op [op ...] [pos]\n";
        std::cout << "  op can be one of:";
        for (auto &[name, _] : opTemplates) {
            std::cout << " " << name;
        }
        std::cout << "\n  or push_#, where # is any valid signed 32-bit integer\n";
        return;
    }

    auto &ops = ctx.eval.Operations();
    size_t pos = ops.size();
    size_t opsEnd = args.size();
    if (args.size() >= 2) {
        try {
            int newPos = std::stoi(args.back());
            if (newPos >= 0) {
                pos = newPos;
            }
            if (pos > ops.size()) {
                pos = ops.size();
            }
            opsEnd--;
        } catch (...) {
            // ignore; likely just an op instead
        }
    }

    std::vector<Operation> newOps;
    for (size_t k = 0; k < opsEnd; k++) {
        auto op = Lowercase(args[k]);
        parseAndAddOp(newOps, op);
    }
    ops.insert(ops.begin() + pos, newOps.begin(), newOps.end());

    util::displayFunc(ops, "=> ", pos, pos + newOps.size());
    std::cout << newOps.size() << " operations inserted\n";
}

void delOp(InteractiveContext &ctx, const std::vector<std::string> &args) {
    if (args.size() < 1) {
        std::cout << "Missing argument: pos\n";
        std::cout << "Usage: delop pos [count = 1]\n";
        return;
    }

    auto &ops = ctx.eval.Operations();
    size_t pos = 0;
    size_t count = 1;
    if (args.size() >= 1) {
        try {
            pos = std::stoi(args[0]);
            if (pos > ops.size()) {
                pos = ops.size();
            }
        } catch (...) {
            std::cout << "Not a valid integer: \"" << args[0] << "\"\n";
            return;
        }
    }
    if (args.size() >= 2) {
        try {
            count = std::stoi(args[1]);
            if (count + pos >= ops.size()) {
                count = ops.size() - pos;
            }
        } catch (...) {
            std::cout << "Not a valid integer: \"" << args[0] << "\"\n";
            return;
        }
    }

    util::displayFunc(ops, "<= ", pos, pos + count);
    ops.erase(ops.begin() + pos, ops.begin() + pos + count);
}

void clear(InteractiveContext &ctx, const std::vector<std::string> &args) {
    ctx.eval.Operations().clear();
}

void save(InteractiveContext &ctx, const std::vector<std::string> &args) {
    if (args.size() < 1) {
        std::cout << "Missing argument: path\n";
        std::cout << "Usage: save path\n";
        return;
    }

    auto &ops = ctx.eval.Operations();
    std::ofstream out{args[0]};
    for (auto &op : ops) {
        out << op.Str() << " ";
    }
    std::cout << "Formula saved to \"" << args[0] << "\"\n";
}

void load(InteractiveContext &ctx, const std::vector<std::string> &args) {
    if (args.size() < 1) {
        std::cout << "Missing argument: path\n";
        std::cout << "Usage: load path\n";
        return;
    }

    auto &ops = ctx.eval.Operations();
    ops.clear();
    std::ifstream in{args[0]};
    std::string opStr;
    while (in >> opStr) {
        auto op = Lowercase(opStr);
        parseAndAddOp(ops, op);
    }
    std::cout << "Formula loaded from \"" << args[0] << "\"\n";
    util::displayFunc(ops, "   ");
}

void dump(InteractiveContext &ctx, const std::vector<std::string> &args) {
    using util::parseNum;

    if (args.size() < 3) {
        std::cout << "Missing arguments\n";
        std::cout << "Usage: dump group width height\n";
        return;
    }

    if (Group group; GroupFromName(args[0], group)) {
        i32 width, height;
        if (!parseNum(args[1], width) || !parseNum(args[2], height)) {
            return;
        }
        auto &dataset = ctx.eval.GetDataSet(group);
        for (auto &dataPoint : dataset) {
            if (dataPoint.width == width && dataPoint.height == height) {
                std::cout << dataPoint.KeyStr() << " = " << dataPoint.expectedOutput << "\n";
            }
        }
    } else {
        std::cout << "Invalid group: \"" << args[0] << "\"\n";
    }
}

} // namespace command

void initCommands() {
    commandFns.clear();
    commands.clear();
    longestCommand = 0;

    auto add = [&](std::initializer_list<std::string> aliases, FnCommand fn, std::string description) {
        size_t length = aliases.size() == 0 ? 0 : (aliases.size() - 1) * 2;
        for (auto &alias : aliases) {
            commandFns.insert({Lowercase(alias), fn});
            length += alias.size();
        }
        longestCommand = std::max(longestCommand, length);
        commands.push_back(CommandDesc{aliases, description});
    };

    add({"q", "exit", "quit"}, command::quit, "Quits the interactive evaluator.");
    add({"h", "help"}, command::help, "Displays this help text.");
    add({"se", "stepeval"}, command::stepEval,
        "Begins evaluating the current formula in step-by-step mode.\n"
        "  Arguments: group width height x y\n"
        "    group: one of LPX, LPY, LNX, LNY, RPX, RPY, RNX, RNY.\n"
        "    width, height, x, y: specify a data point from the group");
    add({"n", "next"}, command::stepNext,
        "Runs the next operation in the current step-by-step evaluation.\n"
        "  Arguments: [count = 1]\n"
        "    count: number of operations to execute.\n");
    add({"g", "go"}, command::stepGo, "Runs the step-by-step evaluation until the end of the formula.");
    add({"r", "reset"}, command::stepReset,
        "Resets the step-by-step evaluation to the beginning and transfers the current formula to it.");
    add({"s", "state"}, command::stepState, "Displays the current state of the step-by-step evaluation.");
    add({"c", "cancel"}, command::stepCancel, "Cancels the current step-by-step evaluation.");
    add({"e", "eval"}, command::eval,
        "Evaluates the current formula against the entire data set or a subset.\n"
        "  Arguments: [group] [width height] [group [width height] ...]\n"
        "    group: one of LPX, LPY, LNX, LNY, RPX, RPY, RNX, RNY.\n"
        "    width and height: specify the size of the slope to evaluate.\n"
        "    If executed without arguments, evaluates all groups and dumps the first ten mismatches of each group.\n"
        "    If executed with the group argument, evaluates that group and dumps the first ten mismatches.\n"
        "    If executed with all arguments, evaluates that specific slope size and dumps all results.\n"
        "    Multiple data set may be evaluated by passing additional groups and optional slope sizes.\n");
    add({"f", "fn", "func"}, command::func, "Displays the current formula.");
    add({"addop"}, command::addOp,
        "Adds one or more operations to the formula.\n"
        "  Arguments: op [op ...] [pos = -1]\n"
        "    op: operator(s) to add\n"
        "    pos: position to insert the operator at (-1 inserts at the end)");
    add({"delop"}, command::delOp,
        "Removes one or more operations from the formula.\n"
        "  Arguments: pos [count = 1]\n"
        "    pos: first operation to remove\n"
        "    count: number of operations to remove");
    add({"clear"}, command::clear, "Clears the formula (erases all operations).");
    add({"save"}, command::save,
        "Saves the current formula to a file.\n"
        "  Arguments: path\n"
        "    path: path to the file where the formula will be saved");
    add({"load"}, command::load,
        "Loads a formula from a file.\n"
        "  Arguments: path\n"
        "    path: path to the file where the formula will be loaded");
    add({"dump"}, command::dump,
        "Dumps a portion of the data set.\n"
        "  Arguments: group width height\n"
        "    group: one of LPX, LPY, LNX, LNY, RPX, RPY, RNX, RNY.\n"
        "    width and height: size of the slope to dump");
}

struct CommandLine {
    std::string cmd;
    std::vector<std::string> args;
};

CommandLine ParseCommandLine(std::string cmdline) {
    CommandLine result;

    // Split command line into tokens (separated by spaces)
    std::istringstream in{cmdline};
    std::vector<std::string> tokens{std::istream_iterator<std::string>{in}, std::istream_iterator<std::string>{}};

    // TODO: implement double-quotes to merge/split consecutive tokens
    // - Double-quotes can be used to group arguments with spaces
    // - Double-double quotes ("") are escaped to double-quotes without quoting text

    // The first token is the command
    if (tokens.size() >= 1) {
        result.cmd = tokens.front();
    }
    if (tokens.size() >= 2) {
        std::copy(tokens.begin() + 1, tokens.end(), std::back_inserter(result.args));
    }

    return result;
}

void runInteractiveEvaluator(std::filesystem::path root) {
    if (commands.empty()) {
        initCommands();
    }

    std::cout << "Please wait, loading data set...";
    InteractiveContext ctx{.eval{loadDataSet(root)}};
    std::cout << " OK\n";

    std::cout << "Interactive evaluator ready\n";
    std::cout << "Type \"help\" for commands\n";

    std::string input;
    while (ctx.running && std::cin) {
        if (ctx.eval.IsStepping()) {
            auto dataPoint = ctx.eval.StepEvalDataPoint();
            if (ctx.eval.StepEvalIndex() < ctx.eval.StepEvalOperations().size()) {
                std::cout << "[step " << ctx.eval.StepEvalIndex() << "] ";
            } else {
                std::cout << "[end] ";
            }
            std::cout << dataPoint.KeyStr();
        }
        std::cout << "> ";
        std::getline(std::cin, input);
        auto cmdLine = ParseCommandLine(input);
        if (commandFns.contains(Lowercase(cmdLine.cmd))) {
            commandFns.at(cmdLine.cmd)(ctx, cmdLine.args);
        } else if (!cmdLine.cmd.empty()) {
            std::cout << "Unknown command \"" << cmdLine.cmd << "\"\n";
        }
    }
    std::cout << "Quitting.\n";
}
