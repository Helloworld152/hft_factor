#pragma once

#include <cstdint>

#include "hft_factor/runtime/internal/factor_graph_context.hpp"

namespace hft_factor {

class FactorNode {
public:
    virtual ~FactorNode() = default;
    virtual const char* factor_id() const = 0;
    virtual bool evaluate(const FactorGraphContext& ctx, double& out) const = 0;
};

inline constexpr uint32_t kFactorPluginAbiVersion = 1;

using CreateFactorNodeFn = FactorNode* (*)();
using DestroyFactorNodeFn = void (*)(FactorNode*);
using FactorPluginAbiVersionFn = uint32_t (*)();

}  // namespace hft_factor
