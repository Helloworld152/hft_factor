#pragma once

#include "hft_factor/runtime/internal/factor_node.hpp"

namespace hft_factor {

class BookImbalanceNode final : public FactorNode {
public:
    const char* factor_id() const override { return "book_imbalance"; }

    bool evaluate(const FactorGraphContext& ctx, double& out) const override {
        const double total_volume =
            static_cast<double>(ctx.state->bid_vol1 + ctx.state->ask_vol1);
        if (total_volume > 0.0) {
            out = static_cast<double>(ctx.state->bid_vol1 - ctx.state->ask_vol1) / total_volume;
            return true;
        }
        return false;
    }
};

}  // namespace hft_factor
