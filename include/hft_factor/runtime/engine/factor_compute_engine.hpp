#pragma once

#include <atomic>
#include <chrono>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <thread>
#include <vector>

#include "hft_common/base/queue.h"
#include "hft_factor/model/factor_value.hpp"
#include "hft_factor/runtime/components/dispatcher.hpp"
#include "hft_factor/runtime/components/publisher.hpp"
#include "hft_factor/runtime/components/source.hpp"
#include "hft_factor/runtime/components/worker.hpp"
#include "hft_factor/runtime/core/config.hpp"
#include "hft_factor/runtime/core/config_loader.hpp"
#include "hft_factor/runtime/internal/tick_task.hpp"

namespace hft_factor {

template <typename SourceT = Source,
          typename DispatcherT = Dispatcher,
          typename WorkerT = Worker,
          typename PublisherT = Publisher>
class FactorComputeEngine {
public:
    FactorComputeEngine() = default;
    ~FactorComputeEngine() { stop(); }

    bool init(const std::string& config_path) {
        Config config {};
        if (!load_config(config_path, config)) {
            return false;
        }
        return init(config);
    }

    bool init(const Config& config) {
        if (config.worker_count == 0 ||
            config.output_capacity == 0 ||
            config.worker_queue_capacity < 2 ||
            config.sink_queue_capacity < 2) {
            return false;
        }

        config_ = config;
        if (!source_.init(config_) || !dispatcher_.init(config_) || !publisher_.init(config_)) {
            return false;
        }

        worker_queues_.clear();
        sink_queues_.clear();
        workers_.clear();
        worker_threads_.clear();

        for (std::size_t i = 0; i < config_.worker_count; ++i) {
            auto worker = std::make_unique<WorkerT>();
            if (!worker->init(config_)) {
                return false;
            }
            workers_.push_back(std::move(worker));
            worker_queues_.push_back(std::make_unique<SpscQueue<TickTask>>(config_.worker_queue_capacity));
            sink_queues_.push_back(std::make_unique<SpscQueue<FactorValue>>(config_.sink_queue_capacity));
        }

        return true;
    }

    const Config& config() const {
        return config_;
    }

    void run() {
        if (worker_queues_.empty()) {
            throw std::runtime_error("engine not initialized");
        }

        running_.store(true, std::memory_order_release);

        publisher_thread_ = std::thread(&FactorComputeEngine::publisher_loop, this);
        for (std::size_t i = 0; i < workers_.size(); ++i) {
            worker_threads_.emplace_back(&FactorComputeEngine::worker_loop, this, i);
        }
        source_thread_ = std::thread(&FactorComputeEngine::source_loop, this);

        source_thread_.join();
        for (auto& thread : worker_threads_) {
            if (thread.joinable()) {
                thread.join();
            }
        }
        worker_threads_.clear();
        if (publisher_thread_.joinable()) {
            publisher_thread_.join();
        }
    }

    void stop() {
        running_.store(false, std::memory_order_release);
    }

private:
    void source_loop() {
        uint64_t next_seq = 0;
        while (running_.load(std::memory_order_acquire)) {
            TickTask task {};
            if (!source_.next(next_seq, task)) {
                continue;
            }
            auto& queue = *worker_queues_[dispatcher_.route(task.tick)];
            while (running_.load(std::memory_order_acquire) && !queue.push(task)) {
                std::this_thread::sleep_for(std::chrono::microseconds(1));
            }
        }
    }

    void worker_loop(std::size_t worker_id) {
        TickTask task {};
        auto& input_queue = *worker_queues_[worker_id];
        auto& output_queue = *sink_queues_[worker_id];
        auto& worker = *workers_[worker_id];

        while (running_.load(std::memory_order_acquire)) {
            if (!input_queue.pop(task)) {
                std::this_thread::sleep_for(std::chrono::microseconds(1));
                continue;
            }

            FactorValue factor = worker.process(task);
            while (running_.load(std::memory_order_acquire) && !output_queue.push(factor)) {
                std::this_thread::sleep_for(std::chrono::microseconds(1));
            }
        }
    }

    void publisher_loop() {
        FactorValue value {};
        while (running_.load(std::memory_order_acquire)) {
            bool consumed = false;
            for (auto& queue : sink_queues_) {
                if (!queue->pop(value)) {
                    continue;
                }
                consumed = true;
                if (!publisher_.publish(value)) {
                    std::this_thread::sleep_for(std::chrono::microseconds(1));
                }
            }
            if (!consumed) {
                std::this_thread::sleep_for(std::chrono::microseconds(1));
            }
        }
    }

    Config config_ {};
    std::atomic<bool> running_ {false};
    SourceT source_ {};
    DispatcherT dispatcher_ {};
    PublisherT publisher_ {};
    std::vector<std::unique_ptr<WorkerT>> workers_;
    std::vector<std::unique_ptr<SpscQueue<TickTask>>> worker_queues_;
    std::vector<std::unique_ptr<SpscQueue<FactorValue>>> sink_queues_;
    std::vector<std::thread> worker_threads_;
    std::thread source_thread_;
    std::thread publisher_thread_;
};

}  // namespace hft_factor
