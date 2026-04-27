#pragma once

#include <algorithm>
#include <array>
#include <cstddef>

template <typename T, std::size_t WindowSize>
class MedianFilter {
  static_assert(WindowSize > 0U, "WindowSize must be greater than zero");

 public:
  MedianFilter() : index_(0U), count_(0U), last_output_{} { buffer_.fill(T{}); }

  void Reset() {
    buffer_.fill(T{});
    index_ = 0U;
    count_ = 0U;
    last_output_ = T{};
  }

  T Apply(const T& sample) {
    buffer_[index_] = sample;
    index_ = (index_ + 1U) % WindowSize;

    if (count_ < WindowSize) {
      ++count_;
    }

    std::array<T, WindowSize> working{};
    for (std::size_t i = 0U; i < count_; ++i) {
      working[i] = buffer_[i];
    }

    std::sort(working.begin(),
              working.begin() + static_cast<std::ptrdiff_t>(count_));

    last_output_ = working[count_ / 2U];
    return last_output_;
  }

  T Get() const { return last_output_; }

  std::size_t Count() const { return count_; }

  constexpr std::size_t Capacity() const { return WindowSize; }

 private:
  std::array<T, WindowSize> buffer_;
  std::size_t index_;
  std::size_t count_;
  T last_output_;
};
