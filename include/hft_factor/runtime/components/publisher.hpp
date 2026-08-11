#pragma once

#include <memory>

#include "hft_common/factor/factor_value.h"
#include "hft_common/ipc/shm_ring_buffer.h"
#include "hft_factor/runtime/core/config.hpp"

namespace hft_factor {

using hft_common::factor::FactorValue;

class Publisher {
public:
    using config_type = Config;

    bool init(const Config& config) {
        writer_ = std::make_unique<hft_common::ipc::ShmRingBuffer<FactorValue>>(
            config.output_shm, true, config.output_capacity);
        return true;
    }

    bool publish(const FactorValue& value) {
        return writer_ && writer_->push(value);
    }

private:
    std::unique_ptr<hft_common::ipc::ShmRingBuffer<FactorValue>> writer_;
};

}  // namespace hft_factor
