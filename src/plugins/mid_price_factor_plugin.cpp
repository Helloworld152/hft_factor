#include "hft_common/factor/factor_plugin.h"

namespace hft_factor {

class MidPriceNode final : public hft_common::factor::FactorNode {
public:
    const char* factor_id() const override { return "mid_price"; }

    bool evaluate(const hft_common::factor::FactorContext& ctx, double& out) const override {
        if (ctx.state->bid1 > 0.0 && ctx.state->ask1 > 0.0) {
            out = ctx.state->curr_mid_price;
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
    return new hft_factor::MidPriceNode();
}

extern "C" void destroy_factor_node(hft_common::factor::FactorNode* node) {
    delete node;
}
