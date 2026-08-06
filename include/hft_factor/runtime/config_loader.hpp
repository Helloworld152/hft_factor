#pragma once

#include <string>

#include "hft_factor/runtime/factor_pipeline_engine.hpp"

namespace hft_factor {

bool load_config(const std::string& config_path, Config& config);

}  // namespace hft_factor
