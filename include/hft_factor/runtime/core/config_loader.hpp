#pragma once

#include <stdexcept>
#include <string>
#include <vector>

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

inline std::vector<std::string> load_string_list(const YAML::Node& node, const char* key) {
    std::vector<std::string> values;
    if (!node[key]) {
        return values;
    }

    const YAML::Node list = node[key];
    if (!list.IsSequence()) {
        throw std::runtime_error(std::string("config key is not sequence: ") + key);
    }

    values.reserve(list.size());
    for (const auto& item : list) {
        values.push_back(item.as<std::string>());
    }
    return values;
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

inline FactorNodeConfig load_factor_node_config(const YAML::Node& node) {
    if (!node["id"]) {
        throw std::runtime_error("factor node missing id");
    }
    if (!node["path"]) {
        throw std::runtime_error("factor node missing path");
    }

    FactorNodeConfig config {};
    config.id = node["id"].as<std::string>();
    config.path = node["path"].as<std::string>();
    config.deps = load_string_list(node, "deps");
    return config;
}

inline void load_factor_graph_config(const YAML::Node& node, Config& config) {
    config.factor_graph.nodes.clear();
    if (!node || !node["nodes"]) {
        return;
    }

    const YAML::Node nodes = node["nodes"];
    if (!nodes.IsSequence()) {
        throw std::runtime_error("factor_graph.nodes must be a sequence");
    }

    config.factor_graph.nodes.reserve(nodes.size());
    for (const auto& item : nodes) {
        config.factor_graph.nodes.push_back(load_factor_node_config(item));
    }
}

}  // namespace detail

inline bool load_config(const std::string& config_path, Config& config) {
    const YAML::Node root = YAML::LoadFile(config_path);
    detail::load_source_config(root["source"] ? root["source"] : YAML::Node(), config);
    detail::load_runtime_config(root["engine"] ? root["engine"] : YAML::Node(), config);
    detail::load_sink_config(root["publisher"] ? root["publisher"] : YAML::Node(), config);
    detail::load_factor_graph_config(root["factor_graph"] ? root["factor_graph"] : YAML::Node(),
                                     config);
    return true;
}

}  // namespace hft_factor
