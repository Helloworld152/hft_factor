#include "hft_factor/runtime/config_loader.hpp"

#include <yaml-cpp/yaml.h>

namespace hft_factor {

namespace {

std::size_t load_size(const YAML::Node& node, const char* key, std::size_t default_value) {
    if (!node[key]) {
        return default_value;
    }
    return node[key].as<std::size_t>();
}

uint64_t load_u64(const YAML::Node& node, const char* key, uint64_t default_value) {
    if (!node[key]) {
        return default_value;
    }
    return node[key].as<uint64_t>();
}

std::string load_string(const YAML::Node& node,
                        const char* key,
                        const std::string& default_value) {
    if (!node[key]) {
        return default_value;
    }
    return node[key].as<std::string>();
}

void load_source_config(const YAML::Node& node, Config& config) {
    config.input_shm = load_string(node, "input_shm", config.input_shm);
}

void load_runtime_config(const YAML::Node& node, Config& config) {
    config.worker_count = load_size(node, "worker_count", config.worker_count);
    config.worker_queue_capacity =
        load_size(node, "worker_queue_capacity", config.worker_queue_capacity);
    config.sink_queue_capacity =
        load_size(node, "sink_queue_capacity", config.sink_queue_capacity);
}

void load_sink_config(const YAML::Node& node, Config& config) {
    config.output_shm = load_string(node, "output_shm", config.output_shm);
    config.output_capacity = load_u64(node, "output_capacity", config.output_capacity);
}

}  // namespace

bool load_config(const std::string& config_path, Config& config) {
    const YAML::Node root = YAML::LoadFile(config_path);
    if (root["source"] || root["engine"] || root["publisher"]) {
        load_source_config(root["source"] ? root["source"] : YAML::Node(), config);
        load_runtime_config(root["engine"] ? root["engine"] : YAML::Node(), config);
        load_sink_config(root["publisher"] ? root["publisher"] : YAML::Node(), config);
        return true;
    }

    // Backward-compatible flat/pipeline config.
    const YAML::Node pipeline = root["pipeline"] ? root["pipeline"] : root;
    load_source_config(pipeline, config);
    load_runtime_config(pipeline, config);
    load_sink_config(pipeline, config);

    return true;
}

}  // namespace hft_factor
