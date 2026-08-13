#include <csignal>
#include <iostream>
#include <string>

#include "hft_factor/runtime/engine/factor_compute_engine_single_thread.hpp"

namespace {

hft_factor::FactorComputeEngineSingleThread<
    hft_factor::Source,
    hft_factor::Worker,
    hft_factor::Publisher>* g_engine = nullptr;

void on_signal(int) {
    std::cerr << "[hft_factor] signal received, stopping single-thread engine" << std::endl;
    if (g_engine) {
        g_engine->stop();
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: hft_factor_single_thread <config.yaml>" << std::endl;
        return 1;
    }

    const std::string config_path = argv[1];
    std::cout << "[hft_factor] starting single-thread engine with config: " << config_path
              << std::endl;
    try {
        hft_factor::FactorComputeEngineSingleThread<
            hft_factor::Source,
            hft_factor::Worker,
            hft_factor::Publisher> engine;
        if (!engine.init(config_path)) {
            std::cerr << "failed to init hft_factor_single_thread" << std::endl;
            return 1;
        }
        std::cout << "[hft_factor] single-thread init succeeded" << std::endl;

        g_engine = &engine;
        std::signal(SIGINT, on_signal);
        std::signal(SIGTERM, on_signal);

        std::cout << "[hft_factor] single-thread engine running" << std::endl;
        engine.run();
        std::cout << "[hft_factor] single-thread engine stopped" << std::endl;
    } catch (const std::exception& ex) {
        std::cerr << "init error: " << ex.what() << std::endl;
        return 1;
    }
    return 0;
}
