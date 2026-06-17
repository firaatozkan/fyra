#pragma once

#include <atomic>
#include <functional>
#include <span>

namespace fyra::interfaces
{

class base_engine
{
public:
    virtual ~base_engine() = default;

    virtual void run(std::stop_token token,
                     std::function<void(std::span<uint8_t>)> audio_callback) const = 0;
};

} // namespace fyra::interfaces