#include <iostream>
#include <print>
#include <thread>
#include <vector>

#include "fyra/audio_engine.hpp"
#include "fyra/sync/spsc_queue.hpp"

std::vector<std::jthread> thread_vector;

int main(int, char**)
{
    try
    {
        auto queue = std::make_shared<
            fyra::sync::spsc_queue<std::vector<uint8_t>, 1024>>();

        auto engine = std::make_shared<fyra::audio_engine>(queue);

        thread_vector.emplace_back(
            [engine](std::stop_token token)
            {
                engine->run(std::move(token));
            });

        // Simple producer to see if we can play WAV file
        thread_vector.emplace_back(
            [queue](std::stop_token token)
            {
                FILE* f = fopen(R"(..\..\..\music\sample_2.wav)", "rb");
                fseek(f, 44, 0);

                uint8_t buffer[4096];

                while (!feof(f) && !token.stop_requested())
                {
                    const auto read = fread(buffer, 1, 4096, f);

                    while (!queue->push(std::vector<uint8_t>{buffer, buffer + read}))
                        std::this_thread::yield();

                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }

                fclose(f);
            });

        std::println("Press Enter to abort...");
        std::cin.get();

        for (auto& i : thread_vector)
            i.request_stop();

        for (auto& i : thread_vector)
            i.join();
    }
    catch (const std::exception& e)
    {
        std::println("{}", e.what());
    }
}
