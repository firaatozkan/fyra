#pragma once

#include "fyra/backends/wasapi/common.hpp"
#include "fyra/interfaces/base_engine.hpp"

namespace fyra::backends::wasapi
{

class engine : public fyra::interfaces::base_engine
{
private:
    using super = fyra::interfaces::base_engine;

public:
    engine();

    ~engine();

    void run(std::stop_token token,
             std::function<void(std::span<uint8_t>)> audio_callback) const override;

private:
    WAVEFORMATEX wave_format;
    uint32_t buffer_frame_count;
    wasapi_ptr<IAudioClient> audio_client;
    wasapi_ptr<IAudioRenderClient> render_client;
};

}; // namespace fyra::backends::wasapi