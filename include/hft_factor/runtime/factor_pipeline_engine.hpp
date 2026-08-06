#pragma once

#include <atomic>
#include <cstddef>
#include <memory>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "hft_common/base/queue.h"
#include "hft_common/ipc/shm_ring_buffer.h"
#include "hft_factor/model/factor_value.hpp"
#include "hft_factor/runtime/factor_calculator.hpp"
#include "hft_factor/runtime/instrument_state.hpp"
#include "hft_factor/runtime/raw_tick_shm_source.hpp"
#include "hft_factor/runtime/tick_task.hpp"

namespace hft_factor {

struct Config {
    std::string input_shm {"CTP_MD"};
    std::string output_shm {"FACTOR_MD"};
    uint64_t output_capacity {1u << 20};
    std::size_t worker_count {2};
    std::size_t worker_queue_capacity {1u << 12};
    std::size_t sink_queue_capacity {1u << 12};
};

class FactorPipelineEngine {
public:
    FactorPipelineEngine() = default;
    ~FactorPipelineEngine();

    bool init(const Config& config);
    void run();
    void stop();

private:
    void source_loop();
    void worker_loop(std::size_t worker_id);
    void sink_loop();
    std::size_t route(const CtpShmTickRecord& tick) const;

    Config config_ {};
    std::atomic<bool> running_ {false};
    std::unique_ptr<RawTickShmSource> source_;
    std::unique_ptr<hft_common::ipc::ShmRingBuffer<FactorValue>> sink_writer_;
    std::vector<std::unique_ptr<SpscQueue<TickTask>>> worker_queues_;
    std::vector<std::unique_ptr<SpscQueue<FactorValue>>> sink_queues_;
    std::vector<std::thread> worker_threads_;
    std::thread source_thread_;
    std::thread sink_thread_;
};

}  // namespace hft_factor
