#pragma once

#include "hft_factor/runtime/internal/factor_node.hpp"

namespace hft_factor {

class MidPriceNode final : public FactorNode {
public:
    const char* factor_id() const override { return "mid_price"; }

    bool evaluate(const FactorGraphContext& ctx, double& out) const override {
        if (ctx.state->bid1 > 0.0 && ctx.state->ask1 > 0.0) {
            out = ctx.state->curr_mid_price;
            return true;
        }
        return false;
    }
};

}  // namespace hft_factor
