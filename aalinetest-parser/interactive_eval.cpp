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
            if (i32 result; m_eval.Eval(dataPoint, GroupPositive(group), GroupLeft(group), result)) {
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
                if (i32 result; m_eval.Eval(dataPoint, GroupPositive(group), GroupLeft(group), result)) {
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
                m_stepEval.BeginEval(dataPoint, GroupPositive(group), GroupLeft(group));
                return true;
            }
        }
        return false;
    }

    void StepEvalReset() {
        if (m_stepEvalActive) {
            m_stepEvalIndex = 0;
            m_stepEval.ops = m_eval.ops;
            m_stepEval.BeginEval(m_stepEvalDataPoint, GroupPositive(m_stepEvalGroup), GroupLeft(m_stepEvalGroup));
        }
    }

    void StepEvalEnd() {
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
        if (result != dataPoint.expectedOutput) {
            if (mismatches.size() < 10) {
                mismatches.push_back(Output{dataPoint, result});
            }
            mismatchCount++;
        }
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
        std::cout << "  " << dataPoint.KeyStr() << "  ->  " << result;
        if (result != dataPoint.expectedOutput) {
            std::cout << " != " << dataPoint.expectedOutput;
        }
        std::cout << "\n";
        if (result != dataPoint.expectedOutput) {
            mismatchCount++;
            totalError += std::abs(result - dataPoint.expectedOutput);
        }
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
    std::filesystem::path lastLoadedFormulaPath;
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

void displayDataSetInfo(InteractiveEvaluator &eval, Group group) {
    auto &dataset = eval.GetDataSet(group);
    if (!dataset.empty()) {
        std::cout << "  " << GroupName(group) << ": " << dataset.size() << " entries\n";
    }
}

void displayDataSets(InteractiveEvaluator &eval) {
    std::cout << "Loaded data sets:\n";
    for (auto group : kGroups) {
        util::displayDataSetInfo(eval, group);
    }
}

void displayFormula(const std::vector<Operation> &ops, std::string marker = "", size_t markerStart = ~0,
                    size_t markerEnd = 0) {
    size_t i = 0;
    std::string spaces(marker.size(), ' ');
    if (ops.empty()) {
        std::cout << spaces << "(empty)\n";
        return;
    }
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
    util::displayFormula(eval.StepEvalOperations(), "=> ", eval.StepEvalIndex(), eval.StepEvalIndex() + 1);
    std::cout << "Stack:\n";
    size_t i = 0;
    for (auto num : eval.StepEvalStack()) {
        std::cout << "   " << (eval.StepEvalStack().size() - i - 1) << ": " << num << "\n";
        i++;
    }
    if (i32 result; eval.StepEvalResult(result)) {
        std::cout << "Current  result: " << result << "\n";
        std::cout << "Expected result: " << dataPoint.expectedOutput << "\n";
    }
}

bool stepBegin(InteractiveEvaluator &eval) {
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

void info(InteractiveContext &ctx, const std::vector<std::string> &args) {
    util::displayDataSets(ctx.eval);

    std::cout << "Current formula:\n";
    util::displayFormula(ctx.eval.Operations(), "   ");

    if (!ctx.lastLoadedFormulaPath.empty()) {
        std::cout << "Formula loaded from " << ctx.lastLoadedFormulaPath << "\n";
    }

    if (ctx.eval.IsStepping()) {
        util::displayStepEvalState(ctx.eval);
    }
}

void stepBegin(InteractiveContext &ctx, const std::vector<std::string> &args) {
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
        util::displayStepEvalState(ctx.eval);
    } else {
        std::cout << "Invalid group: \"" << GroupName(group) << "\"\n";
    }
}

void stepNext(InteractiveContext &ctx, const std::vector<std::string> &args) {
    using util::parseNum;

    if (!ctx.eval.IsStepping()) {
        std::cout << "Not running a step-by-step evaluation. Use the \"stepbegin\" command to start.\n";
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
        if (!util::stepBegin(ctx.eval)) {
            break;
        }
    }

    util::displayStepEvalResult(ctx.eval);
    util::displayStepEvalState(ctx.eval);
}

void stepGo(InteractiveContext &ctx, const std::vector<std::string> &args) {
    using util::parseNum;

    if (!ctx.eval.IsStepping()) {
        std::cout << "Not running a step-by-step evaluation. Use the \"stepbegin\" command to start.\n";
        return;
    }

    for (;;) {
        if (!util::stepBegin(ctx.eval)) {
            break;
        }
    }

    util::displayStepEvalResult(ctx.eval);
    util::displayStepEvalState(ctx.eval);
}

void stepReset(InteractiveContext &ctx, const std::vector<std::string> &args) {
    if (!ctx.eval.IsStepping()) {
        std::cout << "Not running a step-by-step evaluation. Use the \"stepbegin\" command to start.\n";
        return;
    }
    ctx.eval.StepEvalReset();
    std::cout << "Step-by-step evaluation reset.\n";
    util::displayStepEvalState(ctx.eval);
}

void stepState(InteractiveContext &ctx, const std::vector<std::string> &args) {
    if (!ctx.eval.IsStepping()) {
        std::cout << "Not running a step-by-step evaluation. Use the \"stepbegin\" command to start.\n";
        return;
    }
    util::displayStepEvalState(ctx.eval);
}

void stepEnd(InteractiveContext &ctx, const std::vector<std::string> &args) {
    if (!ctx.eval.IsStepping()) {
        std::cout << "Not running a step-by-step evaluation. Use the \"stepbegin\" command to start.\n";
        return;
    }
    ctx.eval.StepEvalEnd();
    std::cout << "Step-by-step evaluation ended.\n";
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
    util::displayFormula(ctx.eval.Operations(), "   ");
}

auto opTemplates = [] {
    std::map<std::string, Operation> templates;
    for (auto op : kOperators) {
        templates.insert({OperatorName(op), {.type = Operation::Type::Operator, .op = op}});
    }
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

    util::displayFormula(ops, "=> ", pos, pos + newOps.size());
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

    util::displayFormula(ops, "<= ", pos, pos + count);
    ops.erase(ops.begin() + pos, ops.begin() + pos + count);
    std::cout << count << " operations removed\n";
}

void clear(InteractiveContext &ctx, const std::vector<std::string> &args) {
    ctx.eval.Operations().clear();
    ctx.lastLoadedFormulaPath.clear();
    std::cout << "Formula cleared.\n";
}

void save(InteractiveContext &ctx, const std::vector<std::string> &args) {
    std::filesystem::path path;
    if (args.size() < 1) {
        if (ctx.lastLoadedFormulaPath.empty()) {
            std::cout << "Missing argument: path\n";
            std::cout << "Usage: save path\n";
            return;
        } else {
            path = ctx.lastLoadedFormulaPath;
        }
    } else {
        path = args[0];
    }

    auto &ops = ctx.eval.Operations();
    std::ofstream out{path};
    for (auto &op : ops) {
        out << op.Str() << " ";
    }
    std::cout << "Formula saved to " << path << "\n";
}

void load(InteractiveContext &ctx, const std::vector<std::string> &args) {
    std::filesystem::path path;
    if (args.size() < 1) {
        if (ctx.lastLoadedFormulaPath.empty()) {
            std::cout << "Missing argument: path\n";
            std::cout << "Usage: load path\n";
            return;
        } else {
            path = ctx.lastLoadedFormulaPath;
        }
    } else {
        path = args[0];
    }

    if (!std::filesystem::exists(path)) {
        std::cout << "File not found: " << path << "\n";
        return;
    }
    if (!std::filesystem::is_regular_file(path)) {
        std::cout << "Not a regular file: " << path << "\n";
        return;
    }

    auto &ops = ctx.eval.Operations();
    ops.clear();
    std::ifstream in{path};
    std::string opStr;
    while (in >> opStr) {
        auto op = Lowercase(opStr);
        parseAndAddOp(ops, op);
    }
    std::cout << "Formula loaded from " << path << "\n";
    util::displayFormula(ops, "   ");
    ctx.lastLoadedFormulaPath = path;
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
    add({"i", "info"}, command::info, "Displays evaluator information.");
    add({"dump"}, command::dump,
        "Dumps a portion of the data set.\n"
        "  Arguments: group width height\n"
        "    group: one of LPX, LPY, LNX, LNY, RPX, RPY, RNX, RNY.\n"
        "    width and height: size of the slope to dump");

    add({"f", "fm", "formula"}, command::func, "Displays the current formula's operations.");
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

    add({"e", "eval"}, command::eval,
        "Evaluates the current formula against the entire data set or a subset.\n"
        "  Arguments: [group] [width height] [group [width height] ...]\n"
        "    group: one of LPX, LPY, LNX, LNY, RPX, RPY, RNX, RNY.\n"
        "    width and height: specify the size of the slope to evaluate.\n"
        "  If executed without arguments, evaluates all groups and dumps the first ten mismatches of each group.\n"
        "  If executed with the group argument, evaluates that group and dumps the first ten mismatches.\n"
        "  If executed with all arguments, evaluates that specific slope size and dumps all results.\n"
        "  Multiple data sets may be evaluated by passing additional groups and optional slope sizes.\n");

    add({"s", "save"}, command::save,
        "Saves the current formula to a file.\n"
        "  Arguments: path\n"
        "    path: path to the file where the formula will be saved.\n"
        "  The path may be omitted if a formula was loaded with \"load\".\n"
        "  If omitted, the formula will be saved to the last loaded file.");
    add({"l", "load"}, command::load,
        "Loads a formula from a file.\n"
        "  Arguments: path\n"
        "    path: path to the file where the formula will be loaded\n"
        "  The path may be omitted if a formula was loaded with this command.\n"
        "  If omitted, the command will reload the formula from the last loaded file, if any.");

    add({"sb", "stepbegin"}, command::stepBegin,
        "Begins evaluating the current formula in step-by-step mode.\n"
        "  Arguments: group width height x y\n"
        "    group: one of LPX, LPY, LNX, LNY, RPX, RPY, RNX, RNY.\n"
        "    width, height, x, y: specify a data point from the group");
    add({"se", "stepend"}, command::stepEnd, "Ends the current step-by-step evaluation.");
    add({"sn", "stepnext"}, command::stepNext,
        "Runs the next operation in the current step-by-step evaluation.\n"
        "  Arguments: [count = 1]\n"
        "    count: number of operations to execute.\n");
    add({"sg", "stepgo"}, command::stepGo, "Runs the step-by-step evaluation until the end of the formula.");
    add({"sr", "stepreset"}, command::stepReset,
        "Resets the step-by-step evaluation to the beginning and transfers the current formula to it.");
    add({"ss", "stepstate"}, command::stepState, "Displays the current state of the step-by-step evaluation.");
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

    std::cout << "Please wait, loading data sets...";
    InteractiveContext ctx{.eval{loadDataSet(root)}};
    std::cout << " OK\n";
    util::displayDataSets(ctx.eval);

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
