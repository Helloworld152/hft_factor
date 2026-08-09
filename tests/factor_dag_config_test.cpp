#include <cmath>
#include <cstdio>
#include <map>
#include <stdexcept>
#include <string>

#include "hft_factor/runtime/components/worker.hpp"

namespace {

hft_factor::TickTask make_tick(uint64_t seq,
                               const char* symbol,
                               double bid,
                               double ask,
                               int bid_volume,
                               int ask_volume) {
    hft_factor::TickTask task {};
    task.seq = seq;
    std::snprintf(task.tick.symbol, sizeof(task.tick.symbol), "%s", symbol);
    task.tick.update_time = 93000000 + seq;
    task.tick.bid_price[0] = bid;
    task.tick.ask_price[0] = ask;
    task.tick.bid_volume[0] = bid_volume;
    task.tick.ask_volume[0] = ask_volume;
    return task;
}

void expect(bool condition, const char* message) {
    if (!condition) {
        throw std::runtime_error(message);
    }
}

std::string plugin_path(const char* name) {
    return std::string(HFT_FACTOR_PLUGIN_DIR) + "/lib" + name + ".so";
}

}  // namespace

int main() {
    hft_factor::Config config {};
    config.factor_graph.nodes = {
        {"mid_price", plugin_path("mid_price_factor_plugin"), {}},
        {"spread", plugin_path("spread_factor_plugin"), {"mid_price"}},
        {"book_imbalance", plugin_path("book_imbalance_factor_plugin"), {}},
        {"ret_1_tick", plugin_path("ret_1_tick_factor_plugin"), {"mid_price"}},
    };

    hft_factor::Worker worker;
    expect(worker.init(config), "worker init failed");

    const auto first = worker.process(make_tick(1, "rb2610", 100.0, 102.0, 10, 5));
    std::map<std::string, double> first_values;
    for (const auto& item : first) {
        first_values[item.factor_id] = item.value;
    }
    expect(std::fabs(first_values["mid_price"] - 101.0) < 1e-9, "mid_price mismatch");
    expect(std::fabs(first_values["spread"] - 2.0) < 1e-9, "spread mismatch");
    expect(std::fabs(first_values["book_imbalance"] - (5.0 / 15.0)) < 1e-9,
           "book imbalance mismatch");

    const auto second = worker.process(make_tick(2, "rb2610", 101.0, 103.0, 12, 6));
    std::map<std::string, double> second_values;
    for (const auto& item : second) {
        second_values[item.factor_id] = item.value;
    }
    expect(std::fabs(second_values["ret_1_tick"] - (102.0 / 101.0 - 1.0)) < 1e-9,
           "ret_1_tick mismatch");
    return 0;
}
