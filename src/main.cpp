#include <csignal>
#include <iostream>
#include <string>

#include "hft_factor/runtime/config_loader.hpp"
#include "hft_factor/runtime/factor_pipeline_engine.hpp"

namespace {

hft_factor::FactorPipelineEngine* g_engine = nullptr;

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

    hft_factor::Config config {};
    const std::string config_path = argv[1];
    try {
        if (!hft_factor::load_config(config_path, config)) {
            std::cerr << "failed to load config: " << config_path << std::endl;
            return 1;
        }
    } catch (const std::exception& ex) {
        std::cerr << "config error: " << ex.what() << std::endl;
        return 1;
    }

    hft_factor::FactorPipelineEngine engine;
    if (!engine.init(config)) {
        std::cerr << "failed to init hft_factor_demo" << std::endl;
        return 1;
    }

    g_engine = &engine;
    std::signal(SIGINT, on_signal);
    std::signal(SIGTERM, on_signal);

    std::cout << "hft_factor_demo starting"
              << " config=" << config_path
              << " input_shm=" << config.input_shm
              << " output_shm=" << config.output_shm
              << " workers=" << config.worker_count
              << std::endl;

    engine.run();
    return 0;
}
