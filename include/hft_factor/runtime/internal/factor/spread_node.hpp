#pragma once

#include "hft_factor/runtime/internal/factor_node.hpp"

namespace hft_factor {

class SpreadNode final : public FactorNode {
public:
    const char* factor_id() const override { return "spread"; }

    bool evaluate(const FactorGraphContext& ctx, double& out) const override {
        if (ctx.state->bid1 > 0.0 && ctx.state->ask1 > 0.0) {
            out = ctx.state->ask1 - ctx.state->bid1;
            return true;
        }
        return false;
    }
};

}  // namespace hft_factor
