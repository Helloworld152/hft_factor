#pragma once

#include "hft_factor/runtime/internal/factor_node.hpp"

namespace hft_factor {

class SpreadNode final : public FactorNode {
public:
    void evaluate(FactorGraphContext& ctx) const override {
        if (ctx.state->bid1 > 0.0 && ctx.state->ask1 > 0.0) {
            ctx.out->spread = ctx.state->ask1 - ctx.state->bid1;
        }
    }
};

}  // namespace hft_factor
