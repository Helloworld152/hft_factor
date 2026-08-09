#pragma once

#include <cstdint>

namespace hft_factor {

struct FactorValue {
    char symbol[32] {};
    uint64_t seq {0};
    uint64_t update_time {0};
    char factor_id[64] {};
    double value {0.0};
};

}  // namespace hft_factor
