#include <csignal>
#include <iostream>
#include <string>

#include "hft_factor/runtime/engine/factor_compute_engine.hpp"

namespace {

hft_factor::FactorComputeEngine<
    hft_factor::Source,
    hft_factor::Dispatcher,
    hft_factor::Worker,
    hft_factor::Publisher>* g_engine = nullptr;

void on_signal(int) {
    if (g_engine) {
        g_engine->stop();
    }
}

}  // namespace

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "usage: hft_factor_demo <config.yaml>" << std::endl;
        return 1;
    }

    const std::string config_path = argv[1];
    try {
        hft_factor::FactorComputeEngine<
            hft_factor::Source,
            hft_factor::Dispatcher,
            hft_factor::Worker,
            hft_factor::Publisher> engine;
        if (!engine.init(config_path)) {
            std::cerr << "failed to init hft_factor_demo" << std::endl;
            return 1;
        }

        g_engine = &engine;
        std::signal(SIGINT, on_signal);
        std::signal(SIGTERM, on_signal);

        engine.run();
    } catch (const std::exception& ex) {
        std::cerr << "init error: " << ex.what() << std::endl;
        return 1;
    }
    return 0;
}
