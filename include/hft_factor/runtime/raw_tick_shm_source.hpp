#pragma once

#include <chrono>
#include <thread>

#include "hft_common/ipc/shm_ring_buffer.h"
#include "hft_factor/model/ctp_shm_tick_record.hpp"
#include "hft_factor/runtime/tick_task.hpp"

namespace hft_factor {

class RawTickShmSource {
public:
    explicit RawTickShmSource(const std::string& shm_name)
        : reader_(shm_name, false) {}

    bool next(uint64_t& next_seq, TickTask& task) {
        const uint64_t latest = reader_.latest_seq();
        if (next_seq == 0) {
            next_seq = latest > 0 ? latest : 1;
        }
        if (next_seq > latest) {
            std::this_thread::sleep_for(std::chrono::microseconds(50));
            return false;
        }
        const auto* tick = reader_.read(next_seq);
        if (!tick) {
            ++next_seq;
            return false;
        }
        task.seq = next_seq++;
        task.tick = *tick;
        return true;
    }

private:
    hft_common::ipc::ShmRingBuffer<CtpShmTickRecord> reader_;
};

}  // namespace hft_factor
