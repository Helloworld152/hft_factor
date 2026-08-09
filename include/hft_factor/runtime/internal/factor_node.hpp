#pragma once

#include "hft_factor/runtime/internal/factor_graph_context.hpp"

namespace hft_factor {

class FactorNode {
public:
    virtual ~FactorNode() = default;
    virtual void evaluate(FactorGraphContext& ctx) const = 0;
};

}  // namespace hft_factor
