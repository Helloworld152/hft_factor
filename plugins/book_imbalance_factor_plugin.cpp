#include "hft_common/factor/factor_plugin.h"

namespace hft_factor {

class BookImbalanceNode final : public hft_common::factor::FactorNode {
public:
    const char* factor_id() const override { return "book_imbalance"; }

    bool evaluate(const hft_common::factor::FactorContext& ctx, double& out) const override {
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

extern "C" uint32_t factor_plugin_abi_version() {
    return hft_common::factor::kFactorPluginAbiVersion;
}

extern "C" hft_common::factor::FactorNode* create_factor_node() {
    return new hft_factor::BookImbalanceNode();
}

extern "C" void destroy_factor_node(hft_common::factor::FactorNode* node) {
    delete node;
}
