#include "interactive_eval.h"

#include "dataset.h"
#include "func.h"

#include <algorithm>
#include <iostream>
#include <map>

std::string Uppercase(std::string str) {
    std::string out;
    std::transform(str.begin(), str.end(), std::back_inserter(out), [](auto c) { return std::toupper(c); });
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
            m_eval.ctx.vars.Apply(dataPoint);
            if (i32 result; m_eval.Eval(result)) {
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

namespace command {

void quit(InteractiveContext &ctx, const std::vector<std::string> &) {
    ctx.running = false;
}

void help(InteractiveContext &, const std::vector<std::string> &) {
    for (auto &[cmd, desc] : commands) {
        std::cout << cmd << " - " << desc.description << "\n";
    }
}

void eval(InteractiveContext &ctx, const std::vector<std::string> &args) {
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

} // namespace command

void initCommands() {
    commands.clear();
    commands.insert({"q", {command::quit, "Quits the interactive evaluator"}});
    commands.insert({"exit", {command::quit, "Quits the interactive evaluator"}});
    commands.insert({"quit", {command::quit, "Quits the interactive evaluator"}});
    commands.insert({"h", {command::help, "Displays this help text"}});
    commands.insert({"help", {command::help, "Displays this help text"}});
    commands.insert({"eval",
                     {command::eval, "Evaluates the current function against the entire data set or a subset (LPX, "
                                     "LPY, LNX, LNY, RPX, RPY, RNX, RNY)"}});
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
        if (commands.contains(cmdLine.cmd)) {
            commands.at(cmdLine.cmd).func(ctx, cmdLine.args);
        } else if (!cmdLine.cmd.empty()) {
            std::cout << "Unknown command \"" << cmdLine.cmd << "\"";
        }
    }
    std::cout << "Quitting.\n";
}
