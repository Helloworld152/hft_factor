#pragma once

#include <cstring>
#include <vector>

#include "hft_common/factor/factor_value.h"
#include "hft_common/factor/instrument_state.h"
#include "hft_common/market_data/market_snapshot.h"

namespace hft_factor {

using hft_common::factor::FactorValue;

class FactorCalculator {
public:
    std::vector<FactorValue> compute(uint64_t seq,
                                     const hft_common::market_data::MarketSnapshot& tick,
                                     const hft_common::factor::InstrumentState& state) const {
        std::vector<FactorValue> out;
        if (state.bid1 > 0.0 && state.ask1 > 0.0) {
            out.push_back(make_value(seq, tick, "mid_price", state.curr_mid_price));
            out.push_back(make_value(seq, tick, "spread", state.ask1 - state.bid1));
        }

        const double total_volume = static_cast<double>(state.bid_vol1 + state.ask_vol1);
        if (total_volume > 0.0) {
            out.push_back(make_value(seq, tick, "book_imbalance",
                                     static_cast<double>(state.bid_vol1 - state.ask_vol1) /
                                         total_volume));
        }

        if (state.initialized && state.last_mid_price > 0.0 && state.curr_mid_price > 0.0) {
            out.push_back(make_value(seq, tick, "ret_1_tick",
                                     state.curr_mid_price / state.last_mid_price - 1.0));
        }
        return out;
    }

private:
    FactorValue make_value(uint64_t seq,
                           const hft_common::market_data::MarketSnapshot& tick,
                           const char* factor_id,
                           double value) const {
        FactorValue out {};
        std::memcpy(out.symbol, tick.symbol, sizeof(tick.symbol));
        (void)seq;
        out.local_ts = tick.local_ts;
        out.exchange_ts = tick.exchange_ts;
        std::snprintf(out.factor_id, sizeof(out.factor_id), "%s", factor_id);
        out.value = value;
        return out;
    }
};

}  // namespace hft_factor
