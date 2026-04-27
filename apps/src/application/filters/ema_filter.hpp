#pragma once

#include <cstdint>
#include <type_traits>

template <typename T>
class EmaFilter {
  static_assert(std::is_arithmetic<T>::value,
                "EmaFilter requires arithmetic type");

 public:
  EmaFilter(uint32_t alpha_num, uint32_t alpha_den)
      : alpha_num_(alpha_num),
        alpha_den_(alpha_den),
        state_{},
        initialized_(false) {}

  void Reset() {
    state_ = T{};
    initialized_ = false;
  }

  void Reset(const T& initial_value) {
    state_ = initial_value;
    initialized_ = true;
  }

  T Apply(const T& sample) {
    if (!initialized_) {
      state_ = sample;
      initialized_ = true;
      return state_;
    }

    const int64_t sample_term =
        static_cast<int64_t>(alpha_num_) * static_cast<int64_t>(sample);

    const int64_t state_term = static_cast<int64_t>(alpha_den_ - alpha_num_) *
                               static_cast<int64_t>(state_);

    state_ = static_cast<T>((sample_term + state_term) /
                            static_cast<int64_t>(alpha_den_));

    return state_;
  }

  T Get() const { return state_; }

  bool IsInitialized() const { return initialized_; }

 private:
  uint32_t alpha_num_;
  uint32_t alpha_den_;
  T state_;
  bool initialized_;
};

