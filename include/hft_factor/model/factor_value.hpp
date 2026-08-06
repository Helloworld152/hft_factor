#pragma once

#include <cstdint>

namespace hft_factor {

struct FactorValue {
    char symbol[32] {};
    uint64_t seq {0};
    uint64_t update_time {0};

    double mid_price {0.0};
    double spread {0.0};
    double book_imbalance {0.0};
    double ret_1_tick {0.0};
};

}  // namespace hft_factor
