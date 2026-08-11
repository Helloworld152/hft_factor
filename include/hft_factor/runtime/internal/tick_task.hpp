#pragma once

#include <cstdint>

#include "hft_common/market_data/market_snapshot.h"

namespace hft_factor {

struct TickTask {
    uint64_t seq {0};
    hft_common::market_data::MarketSnapshot tick {};
};

}  // namespace hft_factor
