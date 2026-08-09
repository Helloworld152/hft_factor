#pragma once

#include "hft_factor/runtime/internal/factor_node.hpp"

namespace hft_factor {

class BookImbalanceNode final : public FactorNode {
public:
    void evaluate(FactorGraphContext& ctx) const override {
        const double total_volume =
            static_cast<double>(ctx.state->bid_vol1 + ctx.state->ask_vol1);
        if (total_volume > 0.0) {
            ctx.out->book_imbalance =
                static_cast<double>(ctx.state->bid_vol1 - ctx.state->ask_vol1) / total_volume;
        }
    }
};

}  // namespace hft_factor
