#pragma once

#include <cstring>

#include "hft_factor/model/ctp_shm_tick_record.hpp"
#include "hft_factor/model/factor_value.hpp"
#include "hft_factor/runtime/instrument_state.hpp"

namespace hft_factor {

class FactorCalculator {
public:
    FactorValue compute(uint64_t seq,
                        const CtpShmTickRecord& tick,
                        const InstrumentState& state) const {
        FactorValue out {};
        std::memcpy(out.symbol, tick.symbol, sizeof(tick.symbol));
        out.seq = seq;
        out.update_time = tick.update_time;

        if (state.bid1 > 0.0 && state.ask1 > 0.0) {
            out.mid_price = state.curr_mid_price;
            out.spread = state.ask1 - state.bid1;
        }

        const double total_volume = static_cast<double>(state.bid_vol1 + state.ask_vol1);
        if (total_volume > 0.0) {
            out.book_imbalance =
                static_cast<double>(state.bid_vol1 - state.ask_vol1) / total_volume;
        }

        if (state.initialized && state.last_mid_price > 0.0 && state.curr_mid_price > 0.0) {
            out.ret_1_tick = state.curr_mid_price / state.last_mid_price - 1.0;
        }
        return out;
    }
};

}  // namespace hft_factor
