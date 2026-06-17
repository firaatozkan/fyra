#include <thread>

#include "fyra/backends/wasapi/engine.hpp"

static constexpr auto REFTIMES_PER_SEC = 10000000;
static constexpr auto REFTIMES_PER_MILLISEC = 10000;

namespace fyra::backends::wasapi
{

engine::engine()
{
    com_call(CoInitializeEx, nullptr, COINIT_MULTITHREADED);

    auto enumerator = com_call_wptr<IMMDeviceEnumerator>(CoCreateInstance,
                                                         __uuidof(MMDeviceEnumerator),
                                                         nullptr,
                                                         CLSCTX_ALL,
                                                         __uuidof(IMMDeviceEnumerator));

    auto device = com_call_wptr(&IMMDeviceEnumerator::GetDefaultAudioEndpoint,
                                *enumerator,
                                eRender,
                                eConsole);

    audio_client = com_call_wptr<IAudioClient>(&IMMDevice::Activate,
                                               *device,
                                               __uuidof(IAudioClient),
                                               CLSCTX_ALL,
                                               nullptr);

    wave_format = WAVEFORMATEX{
        .wFormatTag = WAVE_FORMAT_PCM,
        .nChannels = 2,
        .nSamplesPerSec = 44100,
        .nAvgBytesPerSec = 176400,
        .nBlockAlign = 4,
        .wBitsPerSample = 16,
        .cbSize = 0,
    };

    com_call(&IAudioClient::Initialize,
             *audio_client,
             AUDCLNT_SHAREMODE_EXCLUSIVE,
             0,
             REFTIMES_PER_SEC,
             0,
             &wave_format,
             nullptr);

    buffer_frame_count = com_call_no_alloc(&IAudioClient::GetBufferSize,
                                           *audio_client);

    render_client = com_call_wptr<IAudioRenderClient>(&IAudioClient::GetService,
                                                      *audio_client,
                                                      __uuidof(IAudioRenderClient));
}

engine::~engine()
{
    CoUninitialize();
}

void engine::run(std::stop_token token,
                 std::function<void(std::span<uint8_t>)> audio_callback) const
{
    SetThreadAffinityMask(GetCurrentThread(), 1 << 0);

    const auto buffer = com_call_no_alloc(&IAudioRenderClient::GetBuffer,
                                          *render_client,
                                          buffer_frame_count);

    audio_callback({buffer, buffer_frame_count * wave_format.nBlockAlign});

    com_call(&IAudioRenderClient::ReleaseBuffer,
             *render_client,
             buffer_frame_count,
             0);

    const auto hns_actual = static_cast<uint64_t>(static_cast<double>(REFTIMES_PER_SEC) *
                                                  static_cast<double>(buffer_frame_count) /
                                                  static_cast<double>(wave_format.nSamplesPerSec));

    com_call(&IAudioClient::Start, *audio_client);

    while (!token.stop_requested())
    {
        std::this_thread::sleep_for(
            std::chrono::milliseconds(hns_actual / REFTIMES_PER_MILLISEC / 2));

        const auto padding = com_call_no_alloc(&IAudioClient::GetCurrentPadding,
                                               *audio_client);

        const auto frames_available = buffer_frame_count - padding;

        const auto buffer = com_call_no_alloc(&IAudioRenderClient::GetBuffer,
                                              *render_client,
                                              frames_available);

        audio_callback({buffer, frames_available * wave_format.nBlockAlign});

        com_call(&IAudioRenderClient::ReleaseBuffer,
                 *render_client,
                 frames_available,
                 0);
    }

    std::this_thread::sleep_for(
        std::chrono::milliseconds(hns_actual / REFTIMES_PER_MILLISEC / 2));

    com_call(&IAudioClient::Stop, *audio_client);
}

} // namespace fyra::backends::wasapi