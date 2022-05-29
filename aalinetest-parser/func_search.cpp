#include "func_search.h"

#include <algorithm>

GAFuncSearch::GAFuncSearch(std::filesystem::path root)
    : m_rng(m_rd()) {
    //: m_dataSet{loadXMajorDataSet(root)}

    // Best fitness: 11
    // Function: push_x_width - - push_31 push_5 push_4 dup push_aa_step mul sub push_x_end div_2 dup - - - - - push_x
    // shl sar sub - add shr push_x_start push_9 - mul_width - - - - - - sub mul_height_div_width_aa - push_aa_step - -
    // - div_2 - - add - add

    for (size_t i = 0; i < m_workers.size(); i++) {
        // m_workerStates[i].randomGenerationWeight = 1.0f + i * 0.5f;
        // m_workerStates[i].crossoverPopWeight = 5.0f + i * 1.0f;

        m_workerStates[i].randomMutationChance = 0.10f + i * 0.15f;
        m_workerStates[i].spliceMutationChance = 0.05f + i * 0.10f;
        // m_workerStates[i].reverseMutationChance = 0.05f + i * 0.01f;
        m_workerStates[i].reverseMutationChance = 0.0f;

        // m_workerStates[i].geneEnablePct = 0.4f + i * 0.1f;

        // m_workerStates[i].ComputeParameters();

        m_workers[i] = std::jthread{[&, id = i] {
            while (m_running) {
                NextGeneration(id);
            }
        }};
    }
}

GAFuncSearch::~GAFuncSearch() {
    m_running = false;
}

void GAFuncSearch::NextGeneration(size_t workerId) {
    auto &state = m_workerStates[workerId];

    // Selection
    if (state.reset) {
        // First run; initialize population
        for (auto &chrom : state.population) {
            state.NewChromosome(chrom, m_templateOps);
        }
        state.reset = false;
    }

    // Crossover, mutation and fitness evaluation
    for (size_t idx = 0; idx < state.population.size(); idx++) {
        auto &chrom = state.population[idx];
        if (idx >= state.randomGenStart && idx < state.crossoverStart) {
            state.NewChromosome(chrom, m_templateOps);
            chrom.generation = m_generation;
        } else if (idx >= state.crossoverStart) {
            if (idx < state.crossoverStart + kWorkers) {
                size_t workerIdx = idx - state.crossoverStart;
                auto &sharedState = m_sharedStates[workerIdx];
                state.population[idx] = sharedState.population[!sharedState.popBufferFlip];
            } else {
                if (state.pctDist(state.randomEngine) < 0.5f) {
                    state.OnePointCrossover(chrom, 0, state.crossoverStart - 1);
                } else {
                    state.RandomCrossover(chrom, 0, state.crossoverStart - 1);
                }
                state.RandomizeGenes(chrom, m_templateOps);
                state.SpliceGenes(chrom);
                state.ReverseGenes(chrom);
                state.DisableGenes(chrom);
                state.ShiftChromosome(chrom);
                state.RotateChromosome(chrom);
                state.ShiftGenes(chrom);
            }
            chrom.generation = m_generation;
        }

        state.EvaluateFitness(chrom, m_fixedDataPoints);
        if (chrom.fitness == 0) {
            m_running = false;
        }
    }

    // Share best chromosomes
    // std::shuffle(state.population.begin(), state.population.end(), m_rng);
    std::sort(state.population.begin(), state.population.end());
    auto &shared = m_sharedStates[workerId];
    shared.population[shared.popBufferFlip] = state.population[0];
    shared.popBufferFlip = !shared.popBufferFlip;

    ++m_generation;
    if (m_generation > state.population[0].generation + m_staleGenCount) {
        Reset();
    }
}

void GAFuncSearch::WorkerState::ComputeParameters() {
    rcpTotalWeights = 1.0f / (eliteSelectionWeight + randomGenerationWeight + crossoverPopWeight);
    eliteSelectionPct = eliteSelectionWeight * rcpTotalWeights;
    randomGenerationPct = randomGenerationWeight * rcpTotalWeights;
    // crossoverPopPct = crossoverPopWeight * rcpTotalWeights;

    randomGenStart = population.size() * eliteSelectionPct + 0.5f;
    crossoverStart = population.size() * (eliteSelectionPct + randomGenerationPct) + 0.5f;
}

void GAFuncSearch::WorkerState::NewChromosome(Chromosome &chrom, const std::vector<Operation> &templateOps) {
    // TODO: implement other forms of gene generation
    // - random splicing of small chunks of "sensible" code
    // - incremental sequence (same as brute-force)
    if (pctDist(randomEngine) < 0.999f) {
        // Generate a completely new chromosome 99.9% of the time
        for (auto &gene : chrom.genes) {
            NewGene(gene, templateOps);
        }
    } else {
        /*static constexpr std::array<Operation, 28> kFixedOperations{
            Operation{.type = Operation::Type::Operator, .op = Operator::FracXEnd},
            Operation{.type = Operation::Type::Constant, .constVal = 40},
            Operation{.type = Operation::Type::Operator, .op = Operator::Divide},
            Operation{.type = Operation::Type::Constant, .constVal = 9},
            Operation{.type = Operation::Type::Operator, .op = Operator::Dup},
            Operation{.type = Operation::Type::Operator, .op = Operator::MulWidth},
            Operation{.type = Operation::Type::Operator, .op = Operator::LeftShift},
            Operation{.type = Operation::Type::Constant, .constVal = 36},
            Operation{.type = Operation::Type::Operator, .op = Operator::Multiply},
            Operation{.type = Operation::Type::Operator, .op = Operator::XWidth},
            Operation{.type = Operation::Type::Operator, .op = Operator::PushRight},
            Operation{.type = Operation::Type::Operator, .op = Operator::Multiply},
            Operation{.type = Operation::Type::Operator, .op = Operator::Multiply},
            Operation{.type = Operation::Type::Operator, .op = Operator::PushY},
            Operation{.type = Operation::Type::Operator, .op = Operator::MulWidth},
            Operation{.type = Operation::Type::Operator, .op = Operator::MulHeightDivWidthAA},
            Operation{.type = Operation::Type::Operator, .op = Operator::Subtract},
            Operation{.type = Operation::Type::Operator, .op = Operator::Div2},
            Operation{.type = Operation::Type::Constant, .constVal = 262144},
            Operation{.type = Operation::Type::Operator, .op = Operator::FracXStart},
            Operation{.type = Operation::Type::Constant, .constVal = 262144},
            Operation{.type = Operation::Type::Operator, .op = Operator::Modulo},
            Operation{.type = Operation::Type::Operator, .op = Operator::Subtract},
            Operation{.type = Operation::Type::Operator, .op = Operator::InsertAAFracBits},
            Operation{.type = Operation::Type::Operator, .op = Operator::Add},
            Operation{.type = Operation::Type::Operator, .op = Operator::Add},
            Operation{.type = Operation::Type::Operator, .op = Operator::FracXWidth},
            Operation{.type = Operation::Type::Operator, .op = Operator::Divide},
        };*/

        // TODO: add this:
        // push_1024 push_1023 mul_width shr push_x_start mod push_pos and push_x_start push_x_end push_neg ifelse
        // push_x0 push_5 push_neg mul_width mul sub sub add_1 mul_height_div_width_aa push_aa_step div_2 sub push_1023
        // sub_1 and push_1 push_y mul_height shl push_x_start push_aa_step xor push_2 xor add push_frac_x_end push_18
        // and xor push_10 sub sub_1 sub_1 mod xor

        static constexpr std::array<Operation, 16> kFixedOperations{
            // const i32 startX = m_negative ? XEnd(y) : XStart(y);
            Operation{.type = Operation::Type::Operator, .op = Operator::XStart},
            Operation{.type = Operation::Type::Operator, .op = Operator::XEnd},
            Operation{.type = Operation::Type::Operator, .op = Operator::PushNegative},
            Operation{.type = Operation::Type::Operator, .op = Operator::IfElse},
            // (neg ? xe : xs)

            // const i32 xOffsetOrigin = m_negative ? startX - (m_x0 - m_width) : startX - m_x0;
            Operation{.type = Operation::Type::Operator, .op = Operator::X0},
            Operation{.type = Operation::Type::Operator, .op = Operator::PushWidth},
            Operation{.type = Operation::Type::Operator, .op = Operator::PushNegative},
            Operation{.type = Operation::Type::Operator, .op = Operator::Multiply},
            Operation{.type = Operation::Type::Operator, .op = Operator::Subtract},
            Operation{.type = Operation::Type::Operator, .op = Operator::Subtract},
            // (neg ? xe : xs) - (x0 - w*neg)

            // const i32 coverageBias = ((2 * xOffsetOrigin + 1) * m_height * kAAFracRange) / (2 * m_width);
            Operation{.type = Operation::Type::Operator, .op = Operator::Mul2},
            Operation{.type = Operation::Type::Operator, .op = Operator::Add1},
            Operation{.type = Operation::Type::Operator, .op = Operator::MulHeightDivWidthAA},
            Operation{.type = Operation::Type::Operator, .op = Operator::Div2},
            // (((neg ? xe : xs) - (x0 - w*neg)) * 2 + 1) * h*1024 / (w * 2)

            Operation{.type = Operation::Type::Constant, .constVal = 1023},
            Operation{.type = Operation::Type::Operator, .op = Operator::And},
            // ((((neg ? xe : xs) - (x0 - w*neg)) * 2 + 1) * h*1024 / (w * 2)) & 1023
        };

        size_t spaces = chrom.genes.size() - kFixedOperations.size();
        size_t templatePos = 0;
        for (size_t i = 0; i < chrom.genes.size(); i++) {
            double chanceForSpace = (double)spaces / chrom.genes.size();
            if (templatePos == kFixedOperations.size() || pctDist(randomEngine) < chanceForSpace) {
                chrom.genes[i].enabled = false;
                spaces--;
            } else {
                chrom.genes[i].enabled = true;
                chrom.genes[i].op = kFixedOperations[templatePos++];
            }
        }
    }
}

void GAFuncSearch::WorkerState::OnePointCrossover(Chromosome &chrom, size_t first, size_t last) {
    std::uniform_int<size_t>::param_type popParam{first, last};
    std::uniform_int<size_t>::param_type geneParam{0, chrom.genes.size() - 1};

    size_t firstParentIndex = intDist(randomEngine, popParam);
    size_t secondParentIndex = intDist(randomEngine, popParam);
    size_t crossoverPos = intDist(randomEngine, geneParam);
    auto &firstParent = population[firstParentIndex];
    auto &secondParent = population[secondParentIndex];
    std::copy_n(firstParent.genes.begin(), crossoverPos, chrom.genes.begin());
    std::copy(secondParent.genes.begin() + crossoverPos, secondParent.genes.end(), chrom.genes.begin() + crossoverPos);
}

void GAFuncSearch::WorkerState::RandomCrossover(Chromosome &chrom, size_t first, size_t last) {
    std::uniform_int<size_t>::param_type popParam{first, last};
    std::uniform_int<size_t>::param_type geneParam{0, chrom.genes.size() - 1};

    size_t firstParentIndex = intDist(randomEngine, popParam);
    size_t secondParentIndex = intDist(randomEngine, popParam);
    auto &firstParent = population[firstParentIndex];
    auto &secondParent = population[secondParentIndex];
    for (size_t i = 0; i < chrom.genes.size(); i++) {
        if (pctDist(randomEngine) < 0.5f) {
            chrom.genes[i] = firstParent.genes[i];
        } else {
            chrom.genes[i] = secondParent.genes[i];
        }
    }
}

void GAFuncSearch::WorkerState::NewGene(Gene &gene, const std::vector<Operation> &templateOps) {
    gene.enabled = pctDist(randomEngine) < geneEnablePct;
    if (gene.enabled) {
        std::uniform_int<size_t>::param_type param{0, templateOps.size() - 1};
        size_t index = intDist(randomEngine, param);
        gene.op = templateOps[index];
    }
}

void GAFuncSearch::WorkerState::RandomizeGenes(Chromosome &chrom, const std::vector<Operation> &templateOps) {
    for (auto &gene : chrom.genes) {
        if (pctDist(randomEngine) < randomMutationChance) {
            NewGene(gene, templateOps);
        }
    }
}

void GAFuncSearch::WorkerState::SpliceGenes(Chromosome &chrom) {
    // Swaps genes between two ranges
    if (pctDist(randomEngine) < spliceMutationChance) {
        std::uniform_int<size_t>::param_type param{0, chrom.genes.size() - 1};
        size_t start = intDist(randomEngine, param);
        size_t end = intDist(randomEngine, param);
        size_t pos = intDist(randomEngine, param);

        if (start > end) {
            std::swap(start, end);
        }
        size_t count = end - start + 1;
        pos = std::min(pos, chrom.genes.size() - 1 - (end - start));

        if (pos < start) {
            for (size_t i = 0; i < count; i++) {
                std::swap(chrom.genes[pos + i], chrom.genes[start + i]);
            }
        } else if (pos > start) {
            for (size_t i = 0; i < count; i++) {
                std::swap(chrom.genes[pos + count - 1 - i], chrom.genes[start + count - 1 - i]);
            }
        }
    }
}

void GAFuncSearch::WorkerState::ReverseGenes(Chromosome &chrom) {
    // Reverses a range of genes
    if (pctDist(randomEngine) < reverseMutationChance) {
        std::uniform_int<size_t>::param_type param{0, chrom.genes.size() - 1};
        size_t start = intDist(randomEngine, param);
        size_t end = intDist(randomEngine, param);

        if (start > end) {
            std::swap(start, end);
        }
        std::reverse(chrom.genes.begin() + start, chrom.genes.begin() + end + 1);
    }
}

void GAFuncSearch::WorkerState::DisableGenes(Chromosome &chrom) {
    // Randomly disables genes
    for (auto &gene : chrom.genes) {
        if (gene.enabled && pctDist(randomEngine) < disableMutationChance) {
            gene.enabled = false;
        }
    }
}

void GAFuncSearch::WorkerState::ShiftChromosome(Chromosome &chrom) {
    // Shift all genes in the chromosome left or right.
    // Genes at the edges are discarded/disabled.
    if (pctDist(randomEngine) < shiftChromosomeMutationChance) {
        std::uniform_int<size_t>::param_type param{0, chrom.genes.size() - 1};
        size_t dist = intDist(randomEngine, param);
        bool left = pctDist(randomEngine) < 0.5f;
        if (left) {
            std::shift_left(chrom.genes.begin(), chrom.genes.end(), dist);
        } else { // right
            std::shift_right(chrom.genes.begin(), chrom.genes.end(), dist);
        }
    }
}

void GAFuncSearch::WorkerState::RotateChromosome(Chromosome &chrom) {
    // Rotates all genes in the chromosome
    if (pctDist(randomEngine) < shiftChromosomeMutationChance) {
        std::uniform_int<size_t>::param_type param{0, chrom.genes.size() - 1};
        size_t dist = intDist(randomEngine, param);
        std::rotate(chrom.genes.begin(), chrom.genes.begin() + dist, chrom.genes.end());
    }
}

void GAFuncSearch::WorkerState::ShiftGenes(Chromosome &chrom) {
    // Randomly slides individual genes left or right while preserving the function order
    for (size_t i = 0; i < chrom.genes.size(); i++) {
        if (chrom.genes[i].enabled && pctDist(randomEngine) < shiftGenesMutationChance) {
            size_t left = i;
            size_t right = i;
            while (left > 0) {
                if (!chrom.genes[left - 1].enabled) {
                    --left;
                } else {
                    break;
                }
            }
            while (right > chrom.genes.size()) {
                if (!chrom.genes[right + 1].enabled) {
                    ++right;
                } else {
                    break;
                }
            }
            std::uniform_int<size_t>::param_type param{left, right};
            size_t pos = intDist(randomEngine, param);
            std::swap(chrom.genes[i], chrom.genes[pos]);
        }
    }
}

uint64_t GAFuncSearch::WorkerState::EvaluateFitness(Chromosome &chrom,
                                                    const std::vector<ExtDataPoint> &fixedDataPoints) {
    chrom.fitness = 0;
    chrom.numErrors = 0;

    // Evaluate against the fixed data set
    for (auto &dataPoint : fixedDataPoints) {
        ctx.slope = dataPoint.slope;
        ctx.stack.clear();
        ctx.vars.Apply(dataPoint.dp, dataPoint.left);

        bool valid = true;
        for (auto &gene : chrom.genes) {
            if (!gene.enabled) {
                continue;
            }
            if (!gene.op.Execute(ctx)) {
                valid = false;
                break;
            }
        }
        if (!valid || ctx.stack.empty()) {
            chrom.fitness = std::numeric_limits<uint64_t>::max();
            chrom.numErrors = std::numeric_limits<uint64_t>::max();
            chrom.stackSize = 0;
            return chrom.fitness;
        }
        i32 result = ctx.stack.back();
        /*result &= 1023;
        if (!dataPoint.left) {
            result ^= 1023;
        }*/

        if (result < dataPoint.dp.expectedOutput) {
            chrom.fitness += (i64)dataPoint.dp.expectedOutput - result;
            ++chrom.numErrors;
        } else if (result > dataPoint.upperBound) {
            chrom.fitness += (i64)result - dataPoint.upperBound;
            ++chrom.numErrors;
        }
        /*if (result < dataPoint.dp.expectedOutput || result > dataPoint.upperBound) {
            ++chrom.fitness;
        }*/
    }
    // chrom.fitness *= chrom.numErrors;
    chrom.stackSize = ctx.stack.size();

    // TODO: evaluate against intelligently selected items from the data set
    // - intelligently select entries for the test set
    //   - start with a hand-crafted set that includes the most important corner cases, quirks and glitches
    //     - these should remain fixed in the test set
    //     - 186x185 most importantly
    //   - once the fitness function reaches zero, validate function against entire data set
    //     - add all (or maybe a subset) of failed slopes to check further
    //     - include a generation cutoff on test entries after which they are removed from the test set
    //       - this is set to the current generation + some number of generations
    //       - the intention is to remove entries that haven't failed in a while for performance
    return chrom.fitness;
}
