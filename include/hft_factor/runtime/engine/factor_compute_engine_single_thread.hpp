#pragma once

#include <chrono>
#include <iostream>
#include <stdexcept>
#include <thread>
#include <vector>

#include "hft_common/factor/factor_value.h"
#include "hft_factor/runtime/components/publisher.hpp"
#include "hft_factor/runtime/components/source.hpp"
#include "hft_factor/runtime/components/worker.hpp"
#include "hft_factor/runtime/core/config.hpp"
#include "hft_factor/runtime/core/config_loader.hpp"
#include "hft_factor/runtime/internal/tick_task.hpp"

namespace hft_factor {

using hft_common::factor::FactorValue;

template <typename SourceT = Source, typename WorkerT = Worker, typename PublisherT = Publisher>
class FactorComputeEngineSingleThread {
public:
    FactorComputeEngineSingleThread() = default;
    ~FactorComputeEngineSingleThread() { stop(); }

    bool init(const std::string& config_path) {
        Config config {};
        if (!load_config(config_path, config)) {
            return false;
        }
        return init(config);
    }

    bool init(const Config& config) {
        if (config.output_capacity == 0) {
            std::cerr << "[hft_factor] invalid engine config: output_capacity="
                      << config.output_capacity << std::endl;
            return false;
        }

        std::cout << "[hft_factor] init single-thread engine: output_capacity="
                  << config.output_capacity << std::endl;
        config_ = config;
        if (!source_.init(config_) || !worker_.init(config_) || !publisher_.init(config_)) {
            std::cerr << "[hft_factor] source/worker/publisher init failed" << std::endl;
            return false;
        }

        std::cout << "[hft_factor] single-thread engine init complete" << std::endl;
        return true;
    }

    const Config& config() const { return config_; }

    void run() {
        running_ = true;

        uint64_t next_seq = 0;
        while (running_) {
            TickTask task {};
            if (!source_.next(next_seq, task)) {
                continue;
            }

            std::vector<FactorValue> factors = worker_.process(task);
            for (const auto& factor : factors) {
                while (running_ && !publisher_.publish(factor)) {
                    std::this_thread::sleep_for(std::chrono::microseconds(1));
                }
            }
        }
    }

    void stop() { running_ = false; }

private:
    Config config_ {};
    bool running_ {false};
    SourceT source_ {};
    WorkerT worker_ {};
    PublisherT publisher_ {};
};

}  // namespace hft_factor
