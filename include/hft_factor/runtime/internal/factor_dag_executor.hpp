#pragma once

#include <cstring>
#include <memory>
#include <vector>

#include "hft_factor/runtime/internal/factor/book_imbalance_node.hpp"
#include "hft_factor/runtime/internal/factor_graph_context.hpp"
#include "hft_factor/runtime/internal/factor_node.hpp"
#include "hft_factor/runtime/internal/factor/mid_price_node.hpp"
#include "hft_factor/runtime/internal/factor/ret_1_tick_node.hpp"
#include "hft_factor/runtime/internal/factor/spread_node.hpp"

namespace hft_factor {

class FactorDagExecutor {
public:
    FactorDagExecutor() {
        nodes_.push_back(std::make_unique<MidPriceNode>());
        nodes_.push_back(std::make_unique<SpreadNode>());
        nodes_.push_back(std::make_unique<BookImbalanceNode>());
        nodes_.push_back(std::make_unique<Ret1TickNode>());
    }

    FactorValue execute(uint64_t seq,
                        const CtpShmTickRecord& tick,
                        const InstrumentState& state) const {
        FactorValue out {};
        std::memcpy(out.symbol, tick.symbol, sizeof(tick.symbol));
        out.seq = seq;
        out.update_time = tick.update_time;

        FactorGraphContext ctx(seq, tick, state, out);
        for (const auto& node : nodes_) {
            node->evaluate(ctx);
        }
        return out;
    }

private:
    std::vector<std::unique_ptr<FactorNode>> nodes_;
};

}  // namespace hft_factor
