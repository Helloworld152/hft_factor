#pragma once

#include <cstddef>
#include <functional>
#include <string>

#include "hft_common/market_data/market_snapshot.h"
#include "hft_factor/runtime/core/config.hpp"

namespace hft_factor {

class Dispatcher {
public:
    using config_type = Config;

    bool init(const Config& config) {
        worker_count_ = config.worker_count;
        return worker_count_ > 0;
    }

    std::size_t route(const hft_common::market_data::MarketSnapshot& tick) const {
        const std::string key = symbol_key(tick.symbol, sizeof(tick.symbol));
        return std::hash<std::string>{}(key) % worker_count_;
    }

private:
    static std::string symbol_key(const char* symbol, std::size_t size) {
        std::size_t len = 0;
        while (len < size && symbol[len] != '\0') {
            ++len;
        }
        return std::string(symbol, len);
    }

    std::size_t worker_count_ {0};
};

}  // namespace hft_factor
