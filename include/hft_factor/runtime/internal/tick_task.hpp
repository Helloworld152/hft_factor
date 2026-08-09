#pragma once

#include <cstdint>

#include "hft_common/factor/ctp_shm_tick_record.h"

namespace hft_factor {

struct TickTask {
    uint64_t seq {0};
    hft_common::factor::CtpShmTickRecord tick {};
};

}  // namespace hft_factor
