#include <cmath>
#include <cstring>
#include <stdexcept>

#include "hft_factor/runtime/components/dispatcher.hpp"
#include "hft_factor/runtime/components/worker.hpp"
#include "hft_factor/runtime/engine/factor_compute_engine.hpp"

namespace {

hft_factor::TickTask make_tick(uint64_t seq,
                               const char* symbol,
                               double bid,
                               double ask,
                               int bid_volume,
                               int ask_volume) {
    hft_factor::TickTask task {};
    task.seq = seq;
    std::strncpy(task.tick.symbol, symbol, sizeof(task.tick.symbol) - 1);
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

}  // namespace

int main() {
    hft_factor::Config config {};
    config.worker_count = 4;

    hft_factor::Dispatcher dispatcher;
    expect(dispatcher.init(config), "dispatcher init failed");
    const auto route_a = dispatcher.route(make_tick(1, "rb2610", 100.0, 101.0, 10, 8).tick);
    const auto route_b = dispatcher.route(make_tick(2, "rb2610", 100.5, 101.5, 12, 9).tick);
    expect(route_a == route_b, "dispatcher must route the same symbol to the same worker");

    hft_factor::Worker worker;
    expect(worker.init(config), "worker init failed");

    const auto first = worker.process(make_tick(1, "rb2610", 100.0, 102.0, 10, 5));
    expect(std::fabs(first.mid_price - 101.0) < 1e-9, "mid_price mismatch");
    expect(std::fabs(first.spread - 2.0) < 1e-9, "spread mismatch");
    expect(std::fabs(first.book_imbalance - (5.0 / 15.0)) < 1e-9, "book imbalance mismatch");
    expect(std::fabs(first.ret_1_tick) < 1e-9, "first tick return should be zero");

    const auto second = worker.process(make_tick(2, "rb2610", 101.0, 103.0, 12, 6));
    expect(std::fabs(second.mid_price - 102.0) < 1e-9, "second mid_price mismatch");
    expect(std::fabs(second.ret_1_tick - (102.0 / 101.0 - 1.0)) < 1e-9, "ret_1_tick mismatch");
    return 0;
}
