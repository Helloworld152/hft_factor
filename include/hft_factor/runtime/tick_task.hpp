#pragma once

#include <cstdint>

#include "hft_factor/model/ctp_shm_tick_record.hpp"

namespace hft_factor {

struct TickTask {
    uint64_t seq {0};
    CtpShmTickRecord tick {};
};

}  // namespace hft_factor
