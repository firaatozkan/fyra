#pragma once

#include <atomic>
#include <optional>

namespace fyra::sync
{

template <size_t Align, size_t Size>
struct aligned_storage
{
    alignas(Align) std::byte buffer[Size];
};

template <typename T, size_t N>
class spsc_queue
{
    static_assert((N & (N - 1)) == 0, "N must be power of two");

public:
    ~spsc_queue()
    {
        while (pop())
            ;
    }

    template <typename T_forward>
    bool push(T_forward&& input)
    {
        const auto w = write.load(std::memory_order_relaxed);
        const auto next = (w + 1) & (N - 1);

        if (next == read.load(std::memory_order_acquire))
            return false;

        new (&buffer[w]) T(std::forward<T_forward>(input));
        write.store(next, std::memory_order_release);
        return true;
    }

    std::optional<T> pop()
    {
        const auto r = read.load(std::memory_order_relaxed);
        if (r == write.load(std::memory_order_acquire))
            return std::nullopt;

        T* ptr = std::launder(reinterpret_cast<T*>(&buffer[r]));
        T out = std::move(*ptr);
        ptr->~T();

        read.store((r + 1) & (N - 1), std::memory_order_release);
        return out;
    }

private:
    alignas(std::hardware_destructive_interference_size) std::atomic<size_t> read{0};
    alignas(std::hardware_destructive_interference_size) std::atomic<size_t> write{0};
    aligned_storage<alignof(T), sizeof(T)> buffer[N];
};

} // namespace fyra::data_structures