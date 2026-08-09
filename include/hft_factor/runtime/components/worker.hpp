#pragma once

#include <cstring>
#include <string>
#include <unordered_map>

#include "hft_factor/model/factor_value.hpp"
#include "hft_factor/runtime/core/config.hpp"
#include "hft_factor/runtime/internal/factor_dag_executor.hpp"
#include "hft_factor/runtime/internal/instrument_state.hpp"
#include "hft_factor/runtime/internal/tick_task.hpp"

namespace hft_factor {

class Worker {
public:
    using config_type = Config;

    bool init(const Config&) {
        return true;
    }

    FactorValue process(const TickTask& task) {
        const std::string key(task.tick.symbol, strnlen(task.tick.symbol, sizeof(task.tick.symbol)));
        auto& state = states_[key];
        state.update(task.tick);
        return dag_.execute(task.seq, task.tick, state);
    }

private:
    std::unordered_map<std::string, InstrumentState> states_;
    FactorDagExecutor dag_ {};
};

}  // namespace hft_factor
