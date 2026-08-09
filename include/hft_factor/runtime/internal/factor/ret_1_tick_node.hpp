#pragma once

#include "hft_factor/runtime/internal/factor_node.hpp"

namespace hft_factor {

class Ret1TickNode final : public FactorNode {
public:
    void evaluate(FactorGraphContext& ctx) const override {
        if (ctx.state->initialized && ctx.state->last_mid_price > 0.0 &&
            ctx.state->curr_mid_price > 0.0) {
            ctx.out->ret_1_tick = ctx.state->curr_mid_price / ctx.state->last_mid_price - 1.0;
        }
    }
};

}  // namespace hft_factor
