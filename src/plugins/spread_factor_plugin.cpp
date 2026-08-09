#include "hft_factor/runtime/internal/factor/spread_node.hpp"
#include "hft_factor/runtime/internal/factor_node.hpp"

extern "C" uint32_t factor_plugin_abi_version() {
    return hft_factor::kFactorPluginAbiVersion;
}

extern "C" hft_factor::FactorNode* create_factor_node() {
    return new hft_factor::SpreadNode();
}

extern "C" void destroy_factor_node(hft_factor::FactorNode* node) {
    delete node;
}
