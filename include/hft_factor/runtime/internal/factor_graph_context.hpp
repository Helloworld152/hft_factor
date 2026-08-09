#pragma once

#include <cstdint>

#include "hft_factor/model/ctp_shm_tick_record.hpp"
#include "hft_factor/model/factor_value.hpp"
#include "hft_factor/runtime/internal/instrument_state.hpp"

namespace hft_factor {

struct FactorGraphContext {
    uint64_t seq {0};
    const CtpShmTickRecord* tick {nullptr};
    const InstrumentState* state {nullptr};
    FactorValue* out {nullptr};

    FactorGraphContext(uint64_t seq_in,
                       const CtpShmTickRecord& tick_in,
                       const InstrumentState& state_in,
                       FactorValue& out_in)
        : seq(seq_in), tick(&tick_in), state(&state_in), out(&out_in) {}
};

}  // namespace hft_factor
