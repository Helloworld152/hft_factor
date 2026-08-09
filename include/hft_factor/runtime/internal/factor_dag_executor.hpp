#pragma once

#include <cstring>
#include <dlfcn.h>
#include <functional>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <vector>

#include "hft_common/factor/factor_context.h"
#include "hft_common/factor/factor_plugin.h"
#include "hft_factor/runtime/core/config.hpp"

namespace hft_factor {

struct LoadedFactorNode {
    std::string id;
    std::string path;
    std::unique_ptr<hft_common::factor::FactorNode,
                    std::function<void(hft_common::factor::FactorNode*)>>
        instance;
};

class FactorDagExecutor {
public:
    FactorDagExecutor() = default;

    void init(const Config& config) {
        nodes_.clear();
        if (config.factor_graph.nodes.empty()) {
            throw std::runtime_error("factor_graph.nodes must not be empty");
        }

        std::cout << "[hft_factor] loading factor dag: nodes="
                  << config.factor_graph.nodes.size() << std::endl;
        const auto graph = build_graph(config.factor_graph);
        const auto order = topo_sort(graph);
        nodes_.reserve(order.size());
        for (const auto& id : order) {
            nodes_.push_back(load_plugin(graph.at(id).config));
        }
        log_loaded_graph_once(order);
    }

    std::vector<FactorValue> execute(uint64_t seq,
                                     const hft_common::factor::CtpShmTickRecord& tick,
                                     const hft_common::factor::InstrumentState& state) const {
        std::vector<FactorValue> out;
        out.reserve(nodes_.size());
        hft_common::factor::FactorContext ctx(seq, tick, state);
        for (const auto& node : nodes_) {
            double value = 0.0;
            if (!node.instance->evaluate(ctx, value)) {
                continue;
            }

            FactorValue item {};
            std::memcpy(item.symbol, tick.symbol, sizeof(tick.symbol));
            item.seq = seq;
            item.update_time = tick.update_time;
            std::snprintf(item.factor_id, sizeof(item.factor_id), "%s", node.instance->factor_id());
            item.value = value;
            out.push_back(item);
        }
        return out;
    }

private:
    struct DagNode {
        FactorNodeConfig config;
        std::vector<std::string> downstream;
        std::size_t indegree {0};
    };

    using DagGraph = std::unordered_map<std::string, DagNode>;

    DagGraph build_graph(const FactorGraphConfig& config) const {
        DagGraph graph;
        graph.reserve(config.nodes.size());

        for (const auto& node_cfg : config.nodes) {
            if (node_cfg.id.empty()) {
                throw std::runtime_error("factor node id is empty");
            }
            if (node_cfg.path.empty()) {
                throw std::runtime_error("factor node path is empty: " + node_cfg.id);
            }
            if (graph.find(node_cfg.id) != graph.end()) {
                throw std::runtime_error("duplicate factor node id: " + node_cfg.id);
            }

            DagNode node {};
            node.config = node_cfg;
            graph.emplace(node_cfg.id, std::move(node));
        }

        for (const auto& node_cfg : config.nodes) {
            auto& curr = graph.at(node_cfg.id);
            for (const auto& dep_id : node_cfg.deps) {
                if (dep_id == node_cfg.id) {
                    throw std::runtime_error("self dependency detected: " + node_cfg.id);
                }

                auto dep_it = graph.find(dep_id);
                if (dep_it == graph.end()) {
                    throw std::runtime_error("unknown dependency: " + dep_id +
                                             " for node: " + node_cfg.id);
                }

                ++curr.indegree;
                dep_it->second.downstream.push_back(node_cfg.id);
            }
        }

        return graph;
    }

    std::vector<std::string> topo_sort(const DagGraph& graph) const {
        std::unordered_map<std::string, std::size_t> indegree;
        std::queue<std::string> ready;
        std::vector<std::string> order;
        order.reserve(graph.size());

        for (const auto& [id, node] : graph) {
            indegree.emplace(id, node.indegree);
            if (node.indegree == 0) {
                ready.push(id);
            }
        }

        while (!ready.empty()) {
            std::string id = ready.front();
            ready.pop();
            order.push_back(id);

            const auto& node = graph.at(id);
            for (const auto& next_id : node.downstream) {
                auto& next_indegree = indegree.at(next_id);
                --next_indegree;
                if (next_indegree == 0) {
                    ready.push(next_id);
                }
            }
        }

        if (order.size() != graph.size()) {
            throw std::runtime_error("factor graph contains cycle");
        }

        return order;
    }

    LoadedFactorNode load_plugin(const FactorNodeConfig& config) const {
        void* handle = dlopen(config.path.c_str(), RTLD_NOW | RTLD_LOCAL);
        if (handle == nullptr) {
            throw std::runtime_error("dlopen failed for " + config.path + ": " + dlerror());
        }

        dlerror();
        auto abi_version = reinterpret_cast<hft_common::factor::FactorPluginAbiVersionFn>(
            dlsym(handle, "factor_plugin_abi_version"));
        auto create = reinterpret_cast<hft_common::factor::CreateFactorNodeFn>(
            dlsym(handle, "create_factor_node"));
        auto destroy = reinterpret_cast<hft_common::factor::DestroyFactorNodeFn>(
            dlsym(handle, "destroy_factor_node"));
        const char* symbol_error = dlerror();
        if (symbol_error != nullptr || abi_version == nullptr || create == nullptr ||
            destroy == nullptr) {
            dlclose(handle);
            throw std::runtime_error("plugin symbols missing for " + config.path);
        }

        if (abi_version() != hft_common::factor::kFactorPluginAbiVersion) {
            dlclose(handle);
            throw std::runtime_error("plugin abi mismatch for " + config.path);
        }

        hft_common::factor::FactorNode* raw = create();
        if (raw == nullptr) {
            dlclose(handle);
            throw std::runtime_error("create_factor_node returned null for " + config.path);
        }

        LoadedFactorNode loaded {};
        loaded.id = config.id;
        loaded.path = config.path;
        loaded.instance = {
            raw,
            [handle, destroy](hft_common::factor::FactorNode* node) {
                if (node != nullptr) {
                    destroy(node);
                }
                dlclose(handle);
            },
        };
        return loaded;
    }

    void log_loaded_graph_once(const std::vector<std::string>& order) const {
        static std::once_flag once;
        std::call_once(once, [this, &order]() {
            std::cout << "[hft_factor] factor dag order:";
            for (const auto& id : order) {
                std::cout << ' ' << id;
            }
            std::cout << std::endl;
            for (const auto& node : nodes_) {
                std::cout << "[hft_factor] loaded factor plugin: id=" << node.id
                          << " path=" << node.path << std::endl;
            }
        });
    }

    std::vector<LoadedFactorNode> nodes_;
};

}  // namespace hft_factor
