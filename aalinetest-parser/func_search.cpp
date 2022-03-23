#include "func_search.h"

GAFuncSearch::GAFuncSearch(std::filesystem::path root) {
    //: m_dataSet{loadXMajorDataSet(root)}

    // Best fitness: 11
    // Function: push_x_width - - push_31 push_5 push_4 dup push_aa_step mul sub push_x_end div_2 dup - - - - - push_x
    // shl sar sub - add shr push_x_start push_9 - mul_width - - - - - - sub mul_height_div_width_aa - push_aa_step - -
    // - div_2 - - add - add

    for (size_t i = 0; i < m_workers.size(); i++) {
        /*m_workerStates[i].randomGenerationWeight = 1.0f + i * 0.5f;
        m_workerStates[i].crossoverPopWeight = 5.0f + i * 1.0f;*/

        m_workerStates[i].randomMutationChance = 0.10f + i * 0.15f;
        m_workerStates[i].spliceMutationChance = 0.05f + i * 0.01f;
        m_workerStates[i].reverseMutationChance = 0.05f + i * 0.01f;

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
    if (state.generation == 0) {
        // First run; initialize population
        for (auto &chrom : state.population) {
            state.NewChromosome(chrom, m_templateOps);
        }
    }

    // Crossover, mutation and fitness evaluation
    for (size_t idx = 0; idx < state.population.size(); idx++) {
        auto &chrom = state.population[idx];
        if (idx >= state.randomGenStart && idx < state.crossoverStart) {
            state.NewChromosome(chrom, m_templateOps);
        } else if (idx >= state.crossoverStart) {
            if (idx < state.crossoverStart + kWorkers) {
                // Not 100% thread-safe... we'll get some crossovers for free
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
            }
        }

        state.EvaluateFitness(chrom, m_fixedDataPoints);
        if (chrom.fitness == 0) {
            Stop();
        }
    }

    // Share best chromosomes
    auto &shared = m_sharedStates[workerId];
    std::sort(state.population.begin(), state.population.end());
    shared.population[shared.popBufferFlip] = state.population[0];
    shared.popBufferFlip = !shared.popBufferFlip;

    state.generation++;
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
    for (auto &gene : chrom.genes) {
        NewGene(gene, templateOps);
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
    std::uniform_int<size_t>::param_type param{0, chrom.genes.size() - 1};
    if (pctDist(randomEngine) < spliceMutationChance) {
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
    std::uniform_int<size_t>::param_type param{0, chrom.genes.size() - 1};
    if (pctDist(randomEngine) < reverseMutationChance) {
        size_t start = intDist(randomEngine, param);
        size_t end = intDist(randomEngine, param);

        if (start > end) {
            std::swap(start, end);
        }
        std::reverse(chrom.genes.begin() + start, chrom.genes.begin() + end + 1);
    }
}

uint32_t GAFuncSearch::WorkerState::EvaluateFitness(Chromosome &chrom,
                                                    const std::vector<ExtDataPoint> &fixedDataPoints) {
    chrom.fitness = 0;

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
            break;
        }
        i32 result = ctx.stack.back();
        /*result &= 1023;
        if (!dataPoint.left) {
            result ^= 1023;
        }*/
        chrom.fitness += std::abs(result - dataPoint.dp.expectedOutput);
    }

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
