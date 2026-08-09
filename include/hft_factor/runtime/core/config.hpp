#pragma once

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace hft_factor {

struct FactorNodeConfig {
    std::string id;
    std::string path;
    std::vector<std::string> deps;
};

struct FactorGraphConfig {
    std::vector<FactorNodeConfig> nodes;
};

struct Config {
    std::string input_shm {"CTP_MD"};
    std::string output_shm {"FACTOR_MD"};
    uint64_t output_capacity {1u << 20};
    std::size_t worker_count {2};
    std::size_t worker_queue_capacity {1u << 12};
    std::size_t sink_queue_capacity {1u << 12};
    FactorGraphConfig factor_graph;
};

}  // namespace hft_factor
