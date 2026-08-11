#pragma once

#include <chrono>
#include <memory>
#include <thread>

#include "hft_common/ipc/shm_ring_buffer.h"
#include "hft_common/market_data/market_snapshot.h"
#include "hft_factor/runtime/core/config.hpp"
#include "hft_factor/runtime/internal/tick_task.hpp"

namespace hft_factor {

class Source {
public:
    using config_type = Config;

    bool init(const Config& config) {
        reader_ = std::make_unique<
            hft_common::ipc::ShmRingBuffer<hft_common::market_data::MarketSnapshot>>(
            config.input_shm, false);
        return true;
    }

    bool next(uint64_t& next_seq, TickTask& task) {
        const uint64_t latest = reader_->latest_seq();
        if (next_seq == 0) {
            if (latest == 0) {
                next_seq = 1;
            } else {
                const uint64_t capacity = reader_->get_capacity();
                next_seq = latest >= capacity ? (latest - capacity + 1) : 1;
            }
        }
        if (next_seq > latest) {
            std::this_thread::sleep_for(std::chrono::microseconds(50));
            return false;
        }
        const auto* tick = reader_->read(next_seq);
        if (!tick) {
            ++next_seq;
            return false;
        }
        task.seq = next_seq++;
        task.tick = *tick;
        return true;
    }

private:
    std::unique_ptr<hft_common::ipc::ShmRingBuffer<hft_common::market_data::MarketSnapshot>>
        reader_;
};

}  // namespace hft_factor
