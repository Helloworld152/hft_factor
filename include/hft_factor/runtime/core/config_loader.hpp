#pragma once

#include <string>

#include <yaml-cpp/yaml.h>

#include "hft_factor/runtime/core/config.hpp"

namespace hft_factor {

namespace detail {

inline std::size_t load_size(const YAML::Node& node, const char* key, std::size_t default_value) {
    if (!node[key]) {
        return default_value;
    }
    return node[key].as<std::size_t>();
}

inline uint64_t load_u64(const YAML::Node& node, const char* key, uint64_t default_value) {
    if (!node[key]) {
        return default_value;
    }
    return node[key].as<uint64_t>();
}

inline std::string load_string(const YAML::Node& node,
                               const char* key,
                               const std::string& default_value) {
    if (!node[key]) {
        return default_value;
    }
    return node[key].as<std::string>();
}

inline void load_source_config(const YAML::Node& node, Config& config) {
    config.input_shm = load_string(node, "input_shm", config.input_shm);
}

inline void load_runtime_config(const YAML::Node& node, Config& config) {
    config.worker_count = load_size(node, "worker_count", config.worker_count);
    config.worker_queue_capacity =
        load_size(node, "worker_queue_capacity", config.worker_queue_capacity);
    config.sink_queue_capacity =
        load_size(node, "sink_queue_capacity", config.sink_queue_capacity);
}

inline void load_sink_config(const YAML::Node& node, Config& config) {
    config.output_shm = load_string(node, "output_shm", config.output_shm);
    config.output_capacity = load_u64(node, "output_capacity", config.output_capacity);
}

}  // namespace detail

inline bool load_config(const std::string& config_path, Config& config) {
    const YAML::Node root = YAML::LoadFile(config_path);
    if (root["source"] || root["engine"] || root["publisher"]) {
        detail::load_source_config(root["source"] ? root["source"] : YAML::Node(), config);
        detail::load_runtime_config(root["engine"] ? root["engine"] : YAML::Node(), config);
        detail::load_sink_config(root["publisher"] ? root["publisher"] : YAML::Node(), config);
        return true;
    }

    const YAML::Node pipeline = root["pipeline"] ? root["pipeline"] : root;
    detail::load_source_config(pipeline, config);
    detail::load_runtime_config(pipeline, config);
    detail::load_sink_config(pipeline, config);
    return true;
}

}  // namespace hft_factor
