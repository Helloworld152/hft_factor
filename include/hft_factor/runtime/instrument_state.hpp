#pragma once

#include "hft_factor/model/ctp_shm_tick_record.hpp"

namespace hft_factor {

struct InstrumentState {
    bool initialized {false};
    double last_mid_price {0.0};
    double curr_mid_price {0.0};
    double bid1 {0.0};
    double ask1 {0.0};
    int bid_vol1 {0};
    int ask_vol1 {0};

    void update(const CtpShmTickRecord& tick) {
        last_mid_price = curr_mid_price;
        bid1 = tick.bid_price[0];
        ask1 = tick.ask_price[0];
        bid_vol1 = tick.bid_volume[0];
        ask_vol1 = tick.ask_volume[0];
        if (bid1 > 0.0 && ask1 > 0.0) {
            curr_mid_price = (bid1 + ask1) * 0.5;
        }
        initialized = true;
    }
};

}  // namespace hft_factor
