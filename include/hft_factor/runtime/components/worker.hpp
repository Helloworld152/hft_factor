#pragma once

#include <cstring>
#include <string>
#include <unordered_map>
#include <vector>

#include "hft_common/factor/factor_value.h"
#include "hft_common/factor/instrument_state.h"
#include "hft_factor/runtime/core/config.hpp"
#include "hft_factor/runtime/internal/factor_dag_executor.hpp"
#include "hft_factor/runtime/internal/tick_task.hpp"

namespace hft_factor {

using hft_common::factor::FactorValue;

class Worker {
public:
    using config_type = Config;

    bool init(const Config& config) {
        dag_.init(config);
        return true;
    }

    std::vector<FactorValue> process(const TickTask& task) {
        const std::string key(task.tick.symbol, strnlen(task.tick.symbol, sizeof(task.tick.symbol)));
        auto& state = states_[key];
        state.update(task.tick);
        return dag_.execute(task.seq, task.tick, state);
    }

private:
    std::unordered_map<std::string, hft_common::factor::InstrumentState> states_;
    FactorDagExecutor dag_ {};
};

}  // namespace hft_factor
