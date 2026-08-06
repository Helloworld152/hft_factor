#include "hft_factor/runtime/factor_pipeline_engine.hpp"

#include <chrono>
#include <cstring>
#include <iostream>
#include <thread>
#include <stdexcept>

namespace hft_factor {

namespace {

std::string symbol_key(const char* symbol, std::size_t size) {
    std::size_t len = 0;
    while (len < size && symbol[len] != '\0') {
        ++len;
    }
    return std::string(symbol, len);
}

}  // namespace

FactorPipelineEngine::~FactorPipelineEngine() {
    stop();
}

bool FactorPipelineEngine::init(const Config& config) {
    if (config.worker_count == 0 ||
        config.output_capacity == 0 ||
        config.worker_queue_capacity < 2 ||
        config.sink_queue_capacity < 2) {
        return false;
    }

    config_ = config;
    try {
        source_ = std::make_unique<RawTickShmSource>(config_.input_shm);
        sink_writer_ = std::make_unique<hft_common::ipc::ShmRingBuffer<FactorValue>>(
            config_.output_shm, true, config_.output_capacity);
    } catch (const std::exception& ex) {
        std::cerr << "init failed: " << ex.what() << std::endl;
        return false;
    }

    worker_queues_.clear();
    worker_threads_.clear();
    sink_queues_.clear();
    for (std::size_t i = 0; i < config_.worker_count; ++i) {
        worker_queues_.push_back(std::make_unique<SpscQueue<TickTask>>(config_.worker_queue_capacity));
        sink_queues_.push_back(std::make_unique<SpscQueue<FactorValue>>(config_.sink_queue_capacity));
    }
    return true;
}

void FactorPipelineEngine::run() {
    if (!source_ || !sink_writer_) {
        throw std::runtime_error("engine not initialized");
    }
    running_.store(true, std::memory_order_release);

    sink_thread_ = std::thread(&FactorPipelineEngine::sink_loop, this);
    for (std::size_t i = 0; i < config_.worker_count; ++i) {
        worker_threads_.emplace_back(&FactorPipelineEngine::worker_loop, this, i);
    }
    source_thread_ = std::thread(&FactorPipelineEngine::source_loop, this);

    source_thread_.join();
    for (auto& thread : worker_threads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    if (sink_thread_.joinable()) {
        sink_thread_.join();
    }
}

void FactorPipelineEngine::stop() {
    running_.store(false, std::memory_order_release);
}

void FactorPipelineEngine::source_loop() {
    uint64_t next_seq = 0;
    while (running_.load(std::memory_order_acquire)) {
        TickTask task {};
        if (!source_->next(next_seq, task)) {
            continue;
        }
        auto& queue = *worker_queues_[route(task.tick)];
        while (running_.load(std::memory_order_acquire) && !queue.push(task)) {
            std::this_thread::sleep_for(std::chrono::microseconds(1));
        }
    }
}

void FactorPipelineEngine::worker_loop(std::size_t worker_id) {
    std::unordered_map<std::string, InstrumentState> states;
    FactorCalculator calculator;
    TickTask task {};

    auto& input_queue = *worker_queues_[worker_id];
    auto& output_queue = *sink_queues_[worker_id];
    while (running_.load(std::memory_order_acquire)) {
        if (!input_queue.pop(task)) {
            std::this_thread::sleep_for(std::chrono::microseconds(1));
            continue;
        }
        const std::string key = symbol_key(task.tick.symbol, sizeof(task.tick.symbol));
        auto& state = states[key];
        state.update(task.tick);
        FactorValue factor = calculator.compute(task.seq, task.tick, state);
        while (running_.load(std::memory_order_acquire) && !output_queue.push(factor)) {
            std::this_thread::sleep_for(std::chrono::microseconds(1));
        }
    }
}

void FactorPipelineEngine::sink_loop() {
    FactorValue value {};
    while (running_.load(std::memory_order_acquire)) {
        bool consumed = false;
        for (auto& queue : sink_queues_) {
            if (!queue->pop(value)) {
                continue;
            }
            consumed = true;
            if (!sink_writer_->push(value)) {
                std::cerr << "sink writer dropped factor for symbol=" << value.symbol << std::endl;
            }
        }
        if (!consumed) {
            std::this_thread::sleep_for(std::chrono::microseconds(1));
        }
    }
}

std::size_t FactorPipelineEngine::route(const CtpShmTickRecord& tick) const {
    const std::string key = symbol_key(tick.symbol, sizeof(tick.symbol));
    return std::hash<std::string>{}(key) % config_.worker_count;
}

}  // namespace hft_factor
