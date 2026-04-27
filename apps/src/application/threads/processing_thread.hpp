#pragma once

#include <array>
#include <cstdint>

#include "bms_parameters.hpp"
#include "fault_status.hpp"
#include "median_filter.hpp"
#include "processed_inputs.hpp"
#include "pw_sync/mutex.h"
#include "pw_sync/thread_notification.h"
#include "pw_thread/thread_core.h"
#include "raw_inputs.hpp"

namespace bms::threads {

class ProcessingThreadCore : public pw::thread::ThreadCore {
 public:
  ProcessingThreadCore() = default;

  void Run() override;

  bool PostRawInputs(const bms::domain::RawInputs& raw_inputs);

  bms::domain::ProcessedInputs GetLatestProcessedInputs() const;
  bms::domain::FaultStatus GetLatestFaultStatus() const;

 private:
  static constexpr uint32_t kQueueDepth =
      bms::domain::parameters::kProcessingQueueDepth;

  static constexpr uint32_t kStressScale =
      bms::domain::parameters::kStressScale;
  static constexpr uint32_t kCurrentSamplePeriodMs =
      bms::domain::parameters::kCurrentSamplePeriodMs;

  static constexpr uint32_t kOvervoltageLimitMv =
      bms::domain::parameters::kOvervoltageLimitMv;
  static constexpr int32_t kOvertemperatureLimitMc =
      bms::domain::parameters::kOvertemperatureLimitMc;
  static constexpr int32_t kUndertemperatureLimitMc =
      bms::domain::parameters::kUndertemperatureLimitMc;

  static constexpr uint32_t kCurrentLowThresholdMa =
      bms::domain::parameters::kCurrentLowThresholdMa;
  static constexpr uint32_t kCurrentMediumThresholdMa =
      bms::domain::parameters::kCurrentMediumThresholdMa;
  static constexpr uint32_t kCurrentHighThresholdMa =
      bms::domain::parameters::kCurrentHighThresholdMa;
  static constexpr uint32_t kCurrentCutoffThresholdMa =
      bms::domain::parameters::kCurrentCutoffThresholdMa;

  static constexpr uint32_t kCurrentLowAllowedTimeMs =
      bms::domain::parameters::kCurrentLowAllowedTimeMs;
  static constexpr uint32_t kCurrentMediumAllowedTimeMs =
      bms::domain::parameters::kCurrentMediumAllowedTimeMs;
  static constexpr uint32_t kCurrentHighAllowedTimeMs =
      bms::domain::parameters::kCurrentHighAllowedTimeMs;

  static constexpr uint32_t kCurrentRecoveryThresholdMa =
      bms::domain::parameters::kCurrentRecoveryThresholdMa;

  mutable pw::sync::Mutex mutex_;
  pw::sync::ThreadNotification queue_notification_;

  std::array<bms::domain::RawInputs, kQueueDepth> queue_{};
  uint32_t queue_head_{0U};
  uint32_t queue_tail_{0U};
  uint32_t queue_count_{0U};

  bms::domain::ProcessedInputs processed_inputs_;
  bms::domain::FaultStatus fault_status_;

  MedianFilter<uint32_t, 7U> voltage_filter_;
  MedianFilter<int32_t, 7U> current_filter_;
  MedianFilter<int32_t, 7U> temperature_filter_;

  uint32_t current_stress_{0U};
  bool current_fault_active_{false};

  bool PopRawInputs(bms::domain::RawInputs& raw_inputs);

  void ProcessSample(const bms::domain::RawInputs& raw);
  void EvaluateFaults(const bms::domain::ProcessedInputs& processed);

  uint32_t CalculateCurrentStressIncrement(uint32_t abs_current_ma) const;
  uint32_t AbsoluteCurrentMa(int32_t current_ma) const;
  bool DetectCurrentFault(int32_t current_ma);
};

}  // namespace bms::threads
