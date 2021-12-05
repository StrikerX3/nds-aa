#include "interactive_eval.h"

#include "dataset.h"
#include "func.h"

#include <iostream>

enum class Group { LPX, LPY, LNX, LNY, RPX, RPY, RNX, RNY };

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

void runInteractiveEvaluator(std::filesystem::path root) {
    std::cout << "Please wait, loading data set...";
    InteractiveEvaluator eval{loadDataSet(root)};
    std::cout << " OK\n";

    auto &ops = eval.Operations();
    ops.push_back(Operation{.type = Operation::Type::Operator, .op = Operator::PushX});

    evaluate(eval, Group::LPX);
    evaluate(eval, Group::LPY);
}
