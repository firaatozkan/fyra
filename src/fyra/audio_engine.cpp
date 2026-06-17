#include "fyra/audio_engine.hpp"
#include "fyra/backends/wasapi/engine.hpp"

#include <print>

namespace fyra
{

audio_engine::audio_engine(const std::shared_ptr<fyra::sync::spsc_queue<std::vector<uint8_t>, 1024>>& q)
    : engine_impl(std::make_unique<fyra::backends::wasapi::engine>()), queue(q)
{
}

void audio_engine::run(std::stop_token token) const
{
    size_t leftover_index{0};
    uint8_t leftover_buffer[8192];

    engine_impl->run(std::move(token), [queue = this->queue,
                                        &leftover_index,
                                        &leftover_buffer](std::span<uint8_t> buffer)
                     {
                         size_t offset = 0;

                         if (leftover_index > 0)
                         {
                             const size_t remaining = buffer.size();
                             const size_t copy_size = (std::min)(leftover_index, remaining);

                             std::copy_n(leftover_buffer, copy_size, buffer.begin());

                             offset += copy_size;

                             if (leftover_index > copy_size)
                             {
                                 std::copy_n(&leftover_buffer[leftover_index], leftover_index, leftover_buffer);
                                 leftover_index -= copy_size;
                             }
                             else
                                 leftover_index = 0;
                         }

                         while (offset < buffer.size())
                         {
                             auto chunk_opt = queue->pop();
                             if (!chunk_opt.has_value())
                                 break;

                             const auto chunk = std::move(*chunk_opt);

                             const size_t remaining = buffer.size() - offset;
                             const size_t copy_size = (std::min)(chunk.size(), remaining);

                             std::copy_n(chunk.begin(), copy_size, buffer.begin() + offset);
                             offset += copy_size;

                             if (chunk.size() > copy_size)
                             {
                                 std::copy(chunk.begin() + copy_size,
                                           chunk.end(),
                                           &leftover_buffer[leftover_index]);

                                 leftover_index += (chunk.size() - copy_size);
                             }
                         }

                         if (offset < buffer.size())
                             std::fill(buffer.begin() + offset, buffer.end(), 0);
                     });
}

} // namespace fyra