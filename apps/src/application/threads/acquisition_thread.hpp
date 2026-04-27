#pragma once

#include <array>
#include <cstdint>

#include "bms_parameters.hpp"
#include "pw_chrono/system_clock.h"
#include "pw_chrono/system_timer.h"
#include "pw_sync/mutex.h"
#include "pw_sync/thread_notification.h"
#include "pw_thread/thread_core.h"
#include "raw_inputs.hpp"
#include "raw_inputs_sink.hpp"
#include "sensor_abstraction.hpp"

namespace bms::threads {

class AcquisitionThreadCore
    : public pw::thread::ThreadCore,
      public bms::abstraction::BmsSensorAbstraction::Callback {
 public:
  explicit AcquisitionThreadCore(bms::domain::RawInputsSink& raw_inputs_sink);

  void Run() override;

  void OnSensorDataReady(const bms::abstraction::SensorSample& sample) override;

 private:
  static constexpr uint32_t kSensorQueueSize =
      bms::domain::parameters::kAcquisitionSensorQueueSize;

  static constexpr auto kCurrentPeriod =
      pw::chrono::SystemClock::duration(std::chrono::milliseconds(
          bms::domain::parameters::kCurrentSamplePeriodMs));

  static constexpr auto kVoltagePeriod =
      pw::chrono::SystemClock::duration(std::chrono::milliseconds(
          bms::domain::parameters::kVoltageSamplePeriodMs));

  static constexpr auto kTemperaturePeriod =
      pw::chrono::SystemClock::duration(std::chrono::milliseconds(
          bms::domain::parameters::kTemperatureSamplePeriodMs));

  void OnCurrentTimer(pw::chrono::SystemClock::time_point);
  void OnVoltageTimer(pw::chrono::SystemClock::time_point);
  void OnTemperatureTimer(pw::chrono::SystemClock::time_point);

  void PushSensorSample(const bms::abstraction::SensorSample& sample);
  bool PopSensorSample(bms::abstraction::SensorSample& sample);

  void ApplySensorSample(const bms::abstraction::SensorSample& sample);
  void PublishRawInputs();

  uint32_t NowMs() const;

  bms::domain::RawInputsSink& raw_inputs_sink_;

  bms::abstraction::BmsSensorAbstraction sensor_;

  pw::chrono::SystemTimer current_timer_;
  pw::chrono::SystemTimer voltage_timer_;
  pw::chrono::SystemTimer temperature_timer_;

  pw::sync::ThreadNotification sensor_queue_notification_;

  mutable pw::sync::Mutex mutex_;

  bms::domain::RawInputs raw_inputs_;

  std::array<bms::abstraction::SensorSample, kSensorQueueSize> sensor_queue_{};
  uint32_t sensor_queue_head_{0U};
  uint32_t sensor_queue_tail_{0U};
  uint32_t sensor_queue_count_{0U};

  pw::chrono::SystemClock::time_point start_time_;
};

}  // namespace bms::threads
