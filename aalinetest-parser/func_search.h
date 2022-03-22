#pragma once

#include "dataset.h"
#include "func.h"

#include <atomic>
#include <barrier>
#include <random>
#include <semaphore>
#include <thread>
#include <vector>

struct ExtDataPoint {
    DataPoint dp;
    bool left;
    bool positive;
    Slope slope;
};

// A specialized genetic algorithm for searching functions
class GAFuncSearch {
public:
    static constexpr size_t kNumOps = 48;

    struct Gene {
        Operation op;
        bool enabled;
    };

    struct Chromosome {
        std::array<Gene, kNumOps> genes;
        uint32_t fitness;

        bool operator<(const Chromosome &rhs) const {
            return fitness < rhs.fitness;
        }
    };

    GAFuncSearch(std::filesystem::path root);
    ~GAFuncSearch();

    void SetTemplateOps(const std::vector<Operation> &templateOps) {
        m_templateOps = templateOps;
    }

    void SetFixedDataPoints(const std::vector<ExtDataPoint> &fixedDataPoints) {
        m_fixedDataPoints = fixedDataPoints;
        for (auto &dp : m_fixedDataPoints) {
            if (dp.positive) {
                dp.slope.Setup(0, 0, dp.dp.width, dp.dp.height, dp.left);
            } else {
                dp.slope.Setup(dp.dp.width, 0, 0, dp.dp.height, dp.left);
            }
        }
    }

    uint64_t CurrGeneration() const {
        return m_generation;
    }

    void NextGeneration();

    const std::array<Chromosome, 128> &Population() const {
        return m_population;
    }

    // TODO: serialize and deserialize state

    // TODO: make GA parameters configurable

private:
    static constexpr size_t kWorkers = 5;
    static constexpr bool kWorkOnMainThread = true;

    DataSet m_dataSet;
    std::vector<Operation> m_templateOps;
    std::vector<ExtDataPoint> m_fixedDataPoints;

    std::array<std::jthread, kWorkers> m_workers;
    std::counting_semaphore<kWorkers> m_semaphore;
    std::barrier<> m_barrier;
    std::atomic_size_t m_nextChromosome;
    bool m_running = true;
    Context m_ctx;

    uint64_t m_generation = 0;
    std::array<Chromosome, 128> m_population;

    // Random number generator
    std::random_device m_randomDev;
    std::default_random_engine m_randomEngine;
    std::uniform_int_distribution<size_t> m_intDist;
    std::uniform_real_distribution<float> m_pctDist;
    std::uniform_int_distribution<size_t> m_popDist;

    // Selection parameters
    float m_eliteSelectionWeight = 4.0f;
    float m_randomSelectionWeight = 2.0f;
    float m_randomGenerationWeight = 1.0f;
    float m_crossoverPopWeight = 7.0f;
    size_t m_chromosomesToPreserve = 3;

    // Mutation parameters
    float m_randomMutationChance = 0.10f;
    float m_spliceMutationChance = 0.05f;
    float m_reverseMutationChance = 0.05f;

    // Gene parameters
    float m_geneEnablePct = 0.5f;

    // Computed parameters
    float m_rcpTotalWeights =
        1.0f / (m_eliteSelectionWeight + m_randomSelectionWeight + m_randomGenerationWeight + m_crossoverPopWeight);
    float m_eliteSelectionPct = m_eliteSelectionWeight * m_rcpTotalWeights;
    float m_randomSelectionPct = m_randomSelectionWeight * m_rcpTotalWeights;
    float m_randomGenerationPct = m_randomGenerationWeight * m_rcpTotalWeights;
    float m_crossoverPopPct = m_crossoverPopWeight * m_rcpTotalWeights;

    size_t m_randomIndexStart = m_population.size() * m_eliteSelectionPct + 0.5f;
    size_t m_randomGenStart = m_population.size() * (m_eliteSelectionPct + m_randomSelectionPct) + 0.5f;
    size_t m_crossoverStart =
        m_population.size() * (m_eliteSelectionPct + m_randomSelectionPct + m_randomGenerationPct) + 0.5f;

    void NewChromosome(Chromosome &chrom);
    void OnePointCrossover(Chromosome &chrom, size_t first, size_t last);
    void RandomCrossover(Chromosome &chrom, size_t first, size_t last);

    void NewGene(Gene &gene);

    void RandomizeGenes(Chromosome &chrom, std::uniform_real_distribution<float> pctDist);
    void SpliceGenes(Chromosome &chrom, std::uniform_real_distribution<float> pctDist,
                     std::uniform_int_distribution<size_t> &intDist);
    void ReverseGenes(Chromosome &chrom, std::uniform_real_distribution<float> pctDist,
                      std::uniform_int_distribution<size_t> &intDist);

    uint32_t EvaluateFitness(Chromosome &chrom, Context &ctx);

    void ProcessChromosomes(std::uniform_int_distribution<size_t> &intDist,
                            std::uniform_real_distribution<float> &pctDist, Context &ctx);
};
