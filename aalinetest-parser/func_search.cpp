#include "func_search.h"

GAFuncSearch::GAFuncSearch(std::filesystem::path root)
    : /*m_dataSet{loadXMajorDataSet(root)}
    ,*/
    m_semaphore{0}
    , m_barrier{kWorkers + 1}
    , m_randomEngine{m_randomDev()}
    , m_popDist{0, m_population.size()} {

    // TODO: redesign threading
    // - run entire GA in each thread, including isolated populations and perhaps even different parameters
    // - have threads push some chromosomes (N best, N random) into a public shared pool
    // - have threads consume chromosomes from the public shared pool
    // - minimize locks (e.g. public shared pool has reserved slots for each thread and double-buffering)

    // Best function so far: (fitness = 27)
    //  push_1024 - push_aa_step push_aa_step push_y push_height - div_height - push_frac_x_width push_y
    //  push_frac_x_start push_frac_x_width - insert_aa_frac_bits push_height - shl - - - - - - - div push_y - sub div -
    //  push_31 - shr - push_aa_step div_2 push_x_start - - mul_height_div_width_aa add - - - - add -

    for (size_t i = 0; i < m_workers.size(); i++) {
        m_workers[i] = std::jthread{[&] {
            std::random_device randomDev;
            std::default_random_engine randomEngine;
            std::uniform_int_distribution<size_t> intDist;
            std::uniform_real_distribution<float> pctDist;
            Context ctx;

            for (;;) {
                m_semaphore.acquire();
                if (!m_running) {
                    break;
                }

                ProcessChromosomes(intDist, pctDist, ctx);

                void(m_barrier.arrive());
            }
        }};
    }
}

GAFuncSearch::~GAFuncSearch() {
    m_running = false;
    m_semaphore.release(kWorkers);
}

void GAFuncSearch::NextGeneration() {
    // Selection
    if (m_generation == 0) {
        // First run; initialize population
        for (auto &chrom : m_population) {
            NewChromosome(chrom);
        }
    } else {
        // Elitist selection
        // First m_elitistSelectionPct% entries will contain the best chromosomes
        std::sort(m_population.begin(), m_population.end());
        /*
        // Random selection
        // Next m_randomSelectionPct% entries will contain randomly selected entries
        std::shuffle(m_population.begin() + m_randomIndexStart, m_population.end(), m_randomEngine);

        // Random generation
        // Next m_randomGenerationPct% entries will contain randomly generated entries
        for (size_t i = m_randomGenStart; i < m_crossoverStart; i++) {
            NewChromosome(m_population[i]);
        }

        // Crossover
        // Remaining population will contain entries generated via crossover of two randomly selected parents from the
        // above sections
        for (size_t i = m_crossoverStart; i < m_population.size(); i++) {
            if (m_pctDist(m_randomEngine) < 0.5f) {
                OnePointCrossover(m_population[i], 0, m_crossoverStart - 1);
            } else {
                RandomCrossover(m_population[i], 0, m_crossoverStart - 1);
            }
        }*/
    }

    m_nextChromosome = 0;
    m_semaphore.release(kWorkers);

    if constexpr (kWorkOnMainThread) {
        ProcessChromosomes(m_intDist, m_pctDist, m_ctx);
    }

    m_barrier.arrive_and_wait();

    m_generation++;
}

void GAFuncSearch::NewChromosome(Chromosome &chrom) {
    // TODO: implement other forms of gene generation
    // - random splicing of small chunks of "sensible" code
    // - incremental sequence (same as brute-force)
    for (auto &gene : chrom.genes) {
        NewGene(gene);
    }
}

void GAFuncSearch::OnePointCrossover(Chromosome &chrom, size_t first, size_t last) {
    std::uniform_int<size_t>::param_type popParam{first, last};
    std::uniform_int<size_t>::param_type geneParam{0, chrom.genes.size() - 1};

    size_t firstParentIndex = m_intDist(m_randomEngine, popParam);
    size_t secondParentIndex = m_intDist(m_randomEngine, popParam);
    size_t crossoverPos = m_intDist(m_randomEngine, geneParam);
    auto &firstParent = m_population[firstParentIndex];
    auto &secondParent = m_population[secondParentIndex];
    std::copy_n(firstParent.genes.begin(), crossoverPos, chrom.genes.begin());
    std::copy(secondParent.genes.begin() + crossoverPos, secondParent.genes.end(), chrom.genes.begin() + crossoverPos);
}

void GAFuncSearch::RandomCrossover(Chromosome &chrom, size_t first, size_t last) {
    std::uniform_int<size_t>::param_type popParam{first, last};
    std::uniform_int<size_t>::param_type geneParam{0, chrom.genes.size() - 1};

    size_t firstParentIndex = m_intDist(m_randomEngine, popParam);
    size_t secondParentIndex = m_intDist(m_randomEngine, popParam);
    auto &firstParent = m_population[firstParentIndex];
    auto &secondParent = m_population[secondParentIndex];
    for (size_t i = 0; i < chrom.genes.size(); i++) {
        if (m_pctDist(m_randomEngine) < 0.5f) {
            chrom.genes[i] = firstParent.genes[i];
        } else {
            chrom.genes[i] = secondParent.genes[i];
        }
    }
}

void GAFuncSearch::NewGene(Gene &gene) {
    gene.enabled = m_pctDist(m_randomEngine) < m_geneEnablePct;
    if (gene.enabled) {
        std::uniform_int<size_t>::param_type param{0, m_templateOps.size() - 1};
        size_t index = m_intDist(m_randomEngine, param);
        gene.op = m_templateOps[index];
    }
}

void GAFuncSearch::RandomizeGenes(Chromosome &chrom, std::uniform_real_distribution<float> pctDist) {
    for (auto &gene : chrom.genes) {
        if (m_pctDist(m_randomEngine) < m_randomMutationChance) {
            NewGene(gene);
        }
    }
}

void GAFuncSearch::SpliceGenes(Chromosome &chrom, std::uniform_real_distribution<float> pctDist,
                               std::uniform_int_distribution<size_t> &intDist) {
    std::uniform_int<size_t>::param_type param{0, m_templateOps.size() - 1};
    if (m_pctDist(m_randomEngine) < m_spliceMutationChance) {
        size_t start = m_intDist(m_randomEngine, param);
        size_t end = m_intDist(m_randomEngine, param);
        size_t pos = m_intDist(m_randomEngine, param);

        if (start > end) {
            std::swap(start, end);
        }
        size_t count = end - start + 1;
        pos = std::min(pos, m_templateOps.size() - 1 - (end - start));

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

void GAFuncSearch::ReverseGenes(Chromosome &chrom, std::uniform_real_distribution<float> pctDist,
                                std::uniform_int_distribution<size_t> &intDist) {
    std::uniform_int<size_t>::param_type param{0, m_templateOps.size() - 1};
    if (m_pctDist(m_randomEngine) < m_reverseMutationChance) {
        size_t start = m_intDist(m_randomEngine, param);
        size_t end = m_intDist(m_randomEngine, param);

        if (start > end) {
            std::swap(start, end);
        }
        std::reverse(chrom.genes.begin() + start, chrom.genes.begin() + end + 1);
    }
}

uint32_t GAFuncSearch::EvaluateFitness(Chromosome &chrom, Context &ctx) {
    chrom.fitness = 0;

    // Evaluate against the fixed data set
    for (auto &dataPoint : m_fixedDataPoints) {
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
            chrom.fitness = std::numeric_limits<uint32_t>::max();
            break;
        }
        i32 result = ctx.stack.back();
        result &= 1023;
        if (!dataPoint.left) {
            result ^= 1023;
        }
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

void GAFuncSearch::ProcessChromosomes(std::uniform_int_distribution<size_t> &intDist,
                                      std::uniform_real_distribution<float> &pctDist, Context &ctx) {
    // Crossover, mutation and fitness evaluation
    size_t idx;
    while ((idx = m_nextChromosome++) < m_population.size()) {
        auto &chrom = m_population[idx];
        if (idx >= m_randomGenStart && idx < m_crossoverStart) {
            NewChromosome(chrom);
        } else {
            if (idx >= m_crossoverStart) {
                if (m_pctDist(m_randomEngine) < 0.5f) {
                    OnePointCrossover(chrom, 0, m_crossoverStart - 1);
                } else {
                    RandomCrossover(chrom, 0, m_crossoverStart - 1);
                }
                RandomizeGenes(chrom, pctDist);
                SpliceGenes(chrom, pctDist, intDist);
                ReverseGenes(chrom, pctDist, intDist);
            }
        }
        EvaluateFitness(chrom, ctx);
    }
}
