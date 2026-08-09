#pragma once

#include "hft_factor/runtime/internal/factor_node.hpp"

namespace hft_factor {

class MidPriceNode final : public FactorNode {
public:
    void evaluate(FactorGraphContext& ctx) const override {
        if (ctx.state->bid1 > 0.0 && ctx.state->ask1 > 0.0) {
            ctx.out->mid_price = ctx.state->curr_mid_price;
        }
    }
};

}  // namespace hft_factor
