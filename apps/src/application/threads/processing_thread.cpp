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
  if (processed.voltage_new) {
    voltage_sensor_fault_active_ = !processed.voltage_valid;
    if (processed.voltage_valid) {
      fault_status_.overvoltage = DetectOvervoltage(processed.pack_voltage_mv);
      fault_status_.undervoltage =
          DetectUndervoltage(processed.pack_voltage_mv);
    }
  }

  if (processed.current_new) {
    current_sensor_fault_active_ = !processed.current_valid;
    if (processed.current_valid) {
      fault_status_.overcurrent = DetectCurrentFault(processed.pack_current_ma);
    }
  }

  if (processed.temperature_new) {
    temperature_sensor_fault_active_ = !processed.temperature_valid;
    if (processed.temperature_valid) {
      fault_status_.overtemperature =
          DetectOvertemperature(processed.pack_temperature_mc);
      fault_status_.undertemperature =
          DetectUndertemperature(processed.pack_temperature_mc);
    }
  }

  fault_status_.sensor_fault = voltage_sensor_fault_active_ ||
                               current_sensor_fault_active_ ||
                               temperature_sensor_fault_active_;

}

bool ProcessingThreadCore::DetectOvervoltage(uint32_t voltage_mv) {
  if (!overvoltage_fault_active_) {
    overvoltage_fault_active_ = voltage_mv > kOvervoltageLimitMv;
  } else if (voltage_mv < kOvervoltageRecoveryMv) {
    overvoltage_fault_active_ = false;
  }

  return overvoltage_fault_active_;
}

bool ProcessingThreadCore::DetectUndervoltage(uint32_t voltage_mv) {
  if (!undervoltage_fault_active_) {
    undervoltage_fault_active_ = voltage_mv < kUndervoltageLimitMv;
  } else if (voltage_mv > kUndervoltageRecoveryMv) {
    undervoltage_fault_active_ = false;
  }

  return undervoltage_fault_active_;
}

bool ProcessingThreadCore::DetectOvertemperature(int32_t temperature_mc) {
  if (!overtemperature_fault_active_) {
    overtemperature_fault_active_ = temperature_mc > kOvertemperatureLimitMc;
  } else if (temperature_mc < kOvertemperatureRecoveryMc) {
    overtemperature_fault_active_ = false;
  }

  return overtemperature_fault_active_;
}

bool ProcessingThreadCore::DetectUndertemperature(int32_t temperature_mc) {
  if (!undertemperature_fault_active_) {
    undertemperature_fault_active_ = temperature_mc < kUndertemperatureLimitMc;
  } else if (temperature_mc > kUndertemperatureRecoveryMc) {
    undertemperature_fault_active_ = false;
  }

  return undertemperature_fault_active_;
}

bool ProcessingThreadCore::DetectCurrentFault(int32_t current_ma) {
  const uint32_t abs_current_ma = AbsoluteCurrentMa(current_ma);

  const uint32_t increment = CalculateCurrentStressIncrement(abs_current_ma);

  if (!current_fault_active_) {
    if (current_stress_ > kStressScale - increment) {
      current_stress_ = kStressScale;
    } else {
      current_stress_ += increment;
    }

    if (current_stress_ >= kStressScale) {
      current_fault_active_ = true;
      current_recovery_elapsed_ms_ = 0U;
    }
  } else {
    if (increment > 0U) {
      current_recovery_elapsed_ms_ = 0U;
      current_stress_ = kStressScale;
    } else if (abs_current_ma < kCurrentRecoveryThresholdMa) {
      if (current_recovery_elapsed_ms_ + kCurrentSamplePeriodMs >=
          kCurrentRecoveryDelayMs) {
        current_recovery_elapsed_ms_ = kCurrentRecoveryDelayMs;
      } else {
        current_recovery_elapsed_ms_ += kCurrentSamplePeriodMs;
      }
    } else {
      current_recovery_elapsed_ms_ = 0U;
    }

    if (current_recovery_elapsed_ms_ >= kCurrentRecoveryDelayMs) {
      current_stress_ = 0U;
      current_fault_active_ = false;
      current_recovery_elapsed_ms_ = 0U;
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

  {
    std::lock_guard<pw::sync::Mutex> lock(mutex_);
    processed = processed_inputs_;
  }

  processed.voltage_new = raw.voltage_new;
  processed.current_new = raw.current_new;
  processed.temperature_new = raw.temperature_new;

  if (raw.voltage_new) {
    if (raw.pack_voltage_mv.valid) {
      processed.pack_voltage_mv =
          voltage_filter_.Apply(raw.pack_voltage_mv.value);
    }
    processed.voltage_valid = raw.pack_voltage_mv.valid;
  }

  if (raw.current_new) {
    if (raw.pack_current_ma.valid) {
      processed.pack_current_ma =
          current_filter_.Apply(raw.pack_current_ma.value);
    }
    processed.current_valid = raw.pack_current_ma.valid;
  }

  if (raw.temperature_new) {
    if (raw.pack_temperature_mc.valid) {
      processed.pack_temperature_mc =
          temperature_filter_.Apply(raw.pack_temperature_mc.value);
    }
    processed.temperature_valid = raw.pack_temperature_mc.valid;
  }

  {
    std::lock_guard<pw::sync::Mutex> lock(mutex_);
    processed_inputs_ = processed;
    EvaluateFaults(processed);
    processed_inputs_.voltage_new = false;
    processed_inputs_.current_new = false;
    processed_inputs_.temperature_new = false;
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
