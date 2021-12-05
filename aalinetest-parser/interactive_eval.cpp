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
            if (i32 result; m_eval.Eval(dataPoint, result)) {
                func(dataPoint, result);
            } else {
                return false;
            }
        }
        return true;
    }

    std::vector<Operation> &Operations() {
        return m_eval.ops;
    }

private:
    DataSet m_dataset;
    Evaluator m_eval;

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
};

void evaluate(InteractiveEvaluator &eval, Group group) {
    struct Output {
        DataPoint dataPoint;
        i32 result;
    };

    std::cout << GroupName(group) << ": ";

    std::vector<Output> mismatches;
    mismatches.reserve(10);
    u32 totalError = 0;

    bool valid = eval.Eval(group, [&](const DataPoint &dataPoint, i32 result) {
        if (result != dataPoint.expectedOutput && mismatches.size() < 10) {
            mismatches.push_back(Output{dataPoint, result});
        }
        totalError += std::abs(result - dataPoint.expectedOutput);
    });

    if (valid) {
        if (totalError == 0) {
            std::cout << "Perfect match!\n";
        } else {
            std::cout << "Off by " << totalError << "\n";
            std::cout << "First " << mismatches.size() << " mismatches:\n";
            for (auto &mismatch : mismatches) {
                std::cout << "  " << mismatch.dataPoint.width << "x" << mismatch.dataPoint.height << " @ "
                          << mismatch.dataPoint.x << "x" << mismatch.dataPoint.y << "   " << mismatch.result
                          << " != " << mismatch.dataPoint.expectedOutput << "\n";
            }
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

struct Command {
    FnCommand func;
    std::string description;
};

std::map<std::string, Command> commands;

namespace util {

void displayFunc(std::vector<Operation> &ops, std::string marker = "", size_t markerStart = ~0, size_t markerEnd = 0) {
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

} // namespace util

namespace command {

void quit(InteractiveContext &ctx, const std::vector<std::string> &) {
    ctx.running = false;
}

void help(InteractiveContext &, const std::vector<std::string> &) {
    for (auto &[cmd, desc] : commands) {
        std::cout << cmd << " - " << desc.description << "\n";
    }
}

void evalall(InteractiveContext &ctx, const std::vector<std::string> &args) {
    if (args.empty()) {
        for (auto group : kGroups) {
            evaluate(ctx.eval, group);
        }
    } else {
        for (auto arg : args) {
            if (Group group; GroupFromName(arg, group)) {
                evaluate(ctx.eval, group);
            } else {
                std::cout << "Invalid group: \"" << arg << "\"\n";
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
    std::cout << "Function saved to \"" << args[0] << "\"\n";
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
    std::cout << "Function loaded from \"" << args[0] << "\"\n";
    util::displayFunc(ops, "   ");
}

} // namespace command

void initCommands() {
    commands.clear();

    auto add = [&](std::initializer_list<std::string> aliases, Command command) {
        for (auto &alias : aliases) {
            commands.insert({Lowercase(alias), command});
        }
    };

    add({"q", "exit", "quit"}, {command::quit, "Quits the interactive evaluator."});
    add({"h", "help"}, {command::help, "Displays this help text."});
    add({"evalall"}, {command::evalall, "Evaluates the current function against the entire data set or a subset.\n"
                                        "Valid groups are LPX, LPY, LNX, LNY, RPX, RPY, RNX, RNY."});
    add({"func", "fn"}, {command::func, "Displays the current function."});
    add({"addop"}, {command::addOp, "Adds one or more operations to the function.\n"
                                    "  Arguments: op [op ...] [pos = -1]\n"
                                    "    op: operator(s) to add\n"
                                    "    pos: position to insert the operator at (-1 inserts at the end)"});
    add({"delop"}, {command::delOp, "Removes one or more operations from the function.\n"
                                    "  Arguments: pos [count = 1]\n"
                                    "    pos: first operation to remove\n"
                                    "    count: number of operations to remove"});
    add({"clear"}, {command::clear, "Clears the function (erases all operations)."});
    add({"save"}, {command::save, "Saves the current function to a file.\n"
                                  "  Arguments: path\n"
                                  "    path: path to the file where the function will be saved"});
    add({"load"}, {command::load, "Loads a function from a file.\n"
                                  "  Arguments: path\n"
                                  "    path: path to the file where the function will be loaded"});
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
        std::cout << "> ";
        std::getline(std::cin, input);
        auto cmdLine = ParseCommandLine(input);
        if (commands.contains(Lowercase(cmdLine.cmd))) {
            commands.at(cmdLine.cmd).func(ctx, cmdLine.args);
        } else if (!cmdLine.cmd.empty()) {
            std::cout << "Unknown command \"" << cmdLine.cmd << "\"\n";
        }
    }
    std::cout << "Quitting.\n";
}
