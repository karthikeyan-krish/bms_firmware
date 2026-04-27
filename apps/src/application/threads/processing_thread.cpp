#define PW_LOG_MODULE_NAME "bms-processing-thread"
#define PW_LOG_LEVEL PW_LOG_LEVEL_INFO

#include "processing_thread.hpp"

#include <mutex>

#include "pw_log/log.h"

namespace bms::threads {

bool ProcessingThreadCore::PostRawInputs(
    const bms::domain::RawInputs& raw_inputs) {
  {
    std::lock_guard<pw::sync::Mutex> lock(mutex_);

    if (queue_count_ >= kQueueDepth) {
      queue_tail_ = (queue_tail_ + 1U) % kQueueDepth;
      --queue_count_;
      PW_LOG_WARN("Processing queue full, dropping oldest raw input");
    }

    queue_[queue_head_] = raw_inputs;
    queue_head_ = (queue_head_ + 1U) % kQueueDepth;
    ++queue_count_;
  }

  queue_notification_.release();
  return true;
}

bool ProcessingThreadCore::PopRawInputs(bms::domain::RawInputs& raw_inputs) {
  std::lock_guard<pw::sync::Mutex> lock(mutex_);

  if (queue_count_ == 0U) {
    return false;
  }

  raw_inputs = queue_[queue_tail_];
  queue_tail_ = (queue_tail_ + 1U) % kQueueDepth;
  --queue_count_;

  return true;
}

bms::domain::ProcessedInputs ProcessingThreadCore::GetLatestProcessedInputs()
    const {
  std::lock_guard<pw::sync::Mutex> lock(mutex_);
  return processed_inputs_;
}

bms::domain::FaultStatus ProcessingThreadCore::GetLatestFaultStatus() const {
  std::lock_guard<pw::sync::Mutex> lock(mutex_);
  return fault_status_;
}

void ProcessingThreadCore::EvaluateFaults(
    const bms::domain::ProcessedInputs& processed) {
  fault_status_.sensor_fault = (!processed.voltage_valid) ||
                               (!processed.current_valid) ||
                               (!processed.temperature_valid);

  fault_status_.overvoltage = (processed.voltage_valid &&
                               processed.pack_voltage_mv > kOvervoltageLimitMv);

  fault_status_.overcurrent = DetectCurrentFault(processed.pack_current_ma);

  fault_status_.overtemperature =
      (processed.temperature_valid &&
       processed.pack_temperature_mc > kOvertemperatureLimitMc);

  fault_status_.undertemperature =
      (processed.temperature_valid &&
       processed.pack_temperature_mc < kUndertemperatureLimitMc);
}

bool ProcessingThreadCore::DetectCurrentFault(int32_t current_ma) {
  const uint32_t abs_current_ma = AbsoluteCurrentMa(current_ma);

  if (!current_fault_active_) {
    const uint32_t increment = CalculateCurrentStressIncrement(abs_current_ma);

    if (current_stress_ > kStressScale - increment) {
      current_stress_ = kStressScale;
    } else {
      current_stress_ += increment;
    }

    if (current_stress_ >= kStressScale) {
      current_fault_active_ = true;
    }
  } else {
    // Recovery / hysteresis only after fault is already active.
    if (abs_current_ma < kCurrentRecoveryThresholdMa) {
      current_stress_ = 0U;
      current_fault_active_ = false;
    }
  }

  return current_fault_active_;
}

uint32_t ProcessingThreadCore::AbsoluteCurrentMa(int32_t current_ma) const {
  if (current_ma >= 0) {
    return static_cast<uint32_t>(current_ma);
  }

  return static_cast<uint32_t>(-(static_cast<int64_t>(current_ma)));
}

uint32_t ProcessingThreadCore::CalculateCurrentStressIncrement(
    uint32_t abs_current_ma) const {
  if (abs_current_ma >= kCurrentCutoffThresholdMa) {
    return kStressScale;  // immediate cutoff
  }

  if (abs_current_ma >= kCurrentHighThresholdMa) {
    return (kStressScale * kCurrentSamplePeriodMs) / kCurrentHighAllowedTimeMs;
  }

  if (abs_current_ma >= kCurrentMediumThresholdMa) {
    return (kStressScale * kCurrentSamplePeriodMs) /
           kCurrentMediumAllowedTimeMs;
  }

  if (abs_current_ma >= kCurrentLowThresholdMa) {
    return (kStressScale * kCurrentSamplePeriodMs) / kCurrentLowAllowedTimeMs;
  }

  return 0U;
}

void ProcessingThreadCore::ProcessSample(const bms::domain::RawInputs& raw) {
  bms::domain::ProcessedInputs processed{};

  if (raw.pack_voltage_mv.valid) {
    processed.pack_voltage_mv =
        voltage_filter_.Apply(raw.pack_voltage_mv.value);
    processed.voltage_valid = true;
  }

  if (raw.pack_current_ma.valid) {
    processed.pack_current_ma =
        current_filter_.Apply(raw.pack_current_ma.value);
    processed.current_valid = true;
  }

  if (raw.pack_temperature_mc.valid) {
    processed.pack_temperature_mc =
        temperature_filter_.Apply(raw.pack_temperature_mc.value);
    processed.temperature_valid = true;
  }

  {
    std::lock_guard<pw::sync::Mutex> lock(mutex_);
    processed_inputs_ = processed;
    EvaluateFaults(processed);
  }
}

void ProcessingThreadCore::Run() {
  PW_LOG_INFO("Processing thread started");

  while (true) {
    queue_notification_.acquire();

    bms::domain::RawInputs raw{};

    while (PopRawInputs(raw)) {
      ProcessSample(raw);
    }
  }
}

}  // namespace bms::threads
