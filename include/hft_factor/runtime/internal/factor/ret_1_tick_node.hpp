#pragma once

#include "hft_factor/runtime/internal/factor_node.hpp"

namespace hft_factor {

class Ret1TickNode final : public FactorNode {
public:
    const char* factor_id() const override { return "ret_1_tick"; }

    bool evaluate(const FactorGraphContext& ctx, double& out) const override {
        if (ctx.state->initialized && ctx.state->last_mid_price > 0.0 &&
            ctx.state->curr_mid_price > 0.0) {
            out = ctx.state->curr_mid_price / ctx.state->last_mid_price - 1.0;
            return true;
        }
        return false;
    }
};

}  // namespace hft_factor
