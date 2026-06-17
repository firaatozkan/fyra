#pragma once

#include <memory>
#include <stop_token>
#include <vector>

#include "fyra/interfaces/base_engine.hpp"
#include "fyra/sync/spsc_queue.hpp"

namespace fyra
{

class audio_engine
{
public:
    explicit audio_engine(const std::shared_ptr<fyra::sync::spsc_queue<std::vector<uint8_t>, 1024>>& q);

    ~audio_engine() = default;
    void run(std::stop_token token) const;

private:
    std::unique_ptr<fyra::interfaces::base_engine> engine_impl;
    std::shared_ptr<fyra::sync::spsc_queue<std::vector<uint8_t>, 1024>> queue;
};

} // namespace fyra