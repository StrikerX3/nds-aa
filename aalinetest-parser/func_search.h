#pragma once

#include "dataset.h"
#include "func.h"

#include <atomic>
#include <barrier>
#include <functional>
#include <mutex>
#include <random>
#include <semaphore>
#include <thread>
#include <vector>

struct ExtDataPoint {
    DataPoint dp;
    i32 upperBound;
    bool left;
    bool positive;
    i32 errorWeight = 1;
    Slope slope;
};

// A specialized genetic algorithm for searching functions
class GAFuncSearch {
public:
    static constexpr size_t kNumOps = 64;
    static constexpr size_t kPopSize = 320;
    static constexpr size_t kWorkers = 6;

    struct Gene {
        Operation op;
        bool enabled = false;
    };

    struct Chromosome {
        std::array<Gene, kNumOps> genes;
        uint64_t fitness = std::numeric_limits<uint64_t>::max();
        uint64_t numErrors = 0;
        size_t stackSize = 0;
        uint64_t generation;

        bool operator<(const Chromosome &rhs) const {
            if (fitness < rhs.fitness) {
                return true;
            }
            if (fitness > rhs.fitness) {
                return false;
            }
            if (numErrors < rhs.numErrors) {
                return true;
            }
            if (numErrors > rhs.numErrors) {
                return false;
            }
            return stackSize < rhs.stackSize;
        }
    };

    GAFuncSearch(std::filesystem::path root);
    ~GAFuncSearch();

    void SetResetCallback(std::function<void()> callback) {
        m_onResetCallback = callback;
    }

    void SetTemplateOps(const std::vector<Operation> &templateOps) {
        m_templateOps = templateOps;
    }

    void SetFixedDataPoints(const std::vector<ExtDataPoint> &fixedDataPoints) {
        m_fixedDataPoints = fixedDataPoints;
        for (auto &dp : m_fixedDataPoints) {
            if (dp.positive) {
                dp.slope.Setup(0, 0, dp.dp.width, dp.dp.height, dp.left);
            } else {
                dp.slope.Setup(255, 0, 255 - dp.dp.width, dp.dp.height, dp.left);
            }
        }
    }

    uint64_t CurrGeneration() const {
        return m_generation;
    }

    const Chromosome &BestChromosome() const {
        const Chromosome *best = nullptr;
        for (auto &state : m_sharedStates) {
            if (best == nullptr || state.population[!state.popBufferFlip].fitness < best->fitness) {
                best = &state.population[!state.popBufferFlip];
            }
        }
        return *best;
    }

    std::vector<Chromosome> BestChromosomesHistory() const {
        std::scoped_lock lk{m_bestChromHistoryMutex};
        return m_bestChromHistory;
    }

    uint64_t ResetCount() const {
        return m_resetCount;
    }

    void Stop() {
        m_running = false;
        for (auto &worker : m_workers) {
            worker.join();
        }
    }

    // TODO: serialize and deserialize state

    // TODO: make GA parameters configurable

private:
    DataSet m_dataSet;
    std::vector<Operation> m_templateOps;
    std::vector<ExtDataPoint> m_fixedDataPoints;

    std::array<std::jthread, kWorkers> m_workers;
    bool m_running = true;

    std::random_device m_rd;
    std::mt19937 m_rng;

    void NextGeneration(size_t workerId);

    struct SharedState {
        std::array<Chromosome, 2> population;
        bool popBufferFlip = false;
    };
    std::array<SharedState, kWorkers> m_sharedStates;
    std::atomic_uint64_t m_generation{0};
    std::function<void()> m_onResetCallback = [] {};

    struct ResetBarrierCompletionFunction {
        GAFuncSearch &ref;
        void operator()() noexcept {
            ref.DoReset();
        }
    };
    friend struct ResetBarrierCompletionFunction;

    uint64_t m_staleGenCount = 5000000;
    uint64_t m_resetCount{0};
    std::barrier<ResetBarrierCompletionFunction> m_resetBarrier{kWorkers, ResetBarrierCompletionFunction{*this}};

    std::vector<Chromosome> m_bestChromHistory;
    mutable std::mutex m_bestChromHistoryMutex;

    void Reset() {
        m_resetBarrier.arrive_and_wait();
    }

    void DoReset() {
        std::scoped_lock lk{m_bestChromHistoryMutex};
        m_bestChromHistory.push_back(BestChromosome());
        m_generation = 0;
        ++m_resetCount;
        for (auto &state : m_workerStates) {
            state.reset = true;
        }
        m_sharedStates.fill({});
        m_onResetCallback();
    }

    struct WorkerState {
        WorkerState()
            : randomEngine{randomDev()}
            , popDist{0, population.size()} {}

        bool reset = true;

        std::array<Chromosome, kPopSize> population;

        Context ctx;

        // Random number generator
        std::random_device randomDev;
        std::default_random_engine randomEngine;
        std::uniform_int_distribution<size_t> intDist;
        std::uniform_real_distribution<float> pctDist;
        std::uniform_int_distribution<size_t> popDist;

        // Selection parameters
        float eliteSelectionWeight = 1.0f;
        float randomGenerationWeight = 1.0f;
        float crossoverPopWeight = 5.0f;

        // Mutation parameters
        float randomMutationChance = 0.30f;
        float spliceMutationChance = 0.05f;
        float reverseMutationChance = 0.05f;
        float disableMutationChance = 0.10f;
        float shiftChromosomeMutationChance = 0.15f;
        float rotateChromosomeMutationChance = 0.15f;
        float shiftGenesMutationChance = 0.20f;

        // Gene parameters
        float geneEnablePct = 0.5f;

        // Computed parameters
        float rcpTotalWeights = 1.0f / (eliteSelectionWeight + randomGenerationWeight + crossoverPopWeight);
        float eliteSelectionPct = eliteSelectionWeight * rcpTotalWeights;
        float randomGenerationPct = randomGenerationWeight * rcpTotalWeights;
        // float crossoverPopPct = crossoverPopWeight * rcpTotalWeights;

        size_t randomGenStart = population.size() * eliteSelectionPct + 0.5f;
        size_t crossoverStart = population.size() * (eliteSelectionPct + randomGenerationPct) + 0.5f;

        void ComputeParameters();

        void NewChromosome(Chromosome &chrom, const std::vector<Operation> &templateOps);
        void OnePointCrossover(Chromosome &chrom, size_t first, size_t last);
        void RandomCrossover(Chromosome &chrom, size_t first, size_t last);

        void NewGene(Gene &gene, const std::vector<Operation> &templateOps);

        void RandomizeGenes(Chromosome &chrom, const std::vector<Operation> &templateOps);
        void SpliceGenes(Chromosome &chrom);
        void ReverseGenes(Chromosome &chrom);
        void DisableGenes(Chromosome &chrom);
        void ShiftChromosome(Chromosome &chrom);
        void RotateChromosome(Chromosome &chrom);
        void ShiftGenes(Chromosome &chrom);

        uint64_t EvaluateFitness(Chromosome &chrom, const std::vector<ExtDataPoint> &fixedDataPoints);
    };
    std::array<WorkerState, kWorkers> m_workerStates;
};
