#define PW_LOG_MODULE_NAME "bms-acquisition-thread"
#define PW_LOG_LEVEL PW_LOG_LEVEL_INFO

#include "acquisition_thread.hpp"

#include <chrono>
#include <mutex>

#include "pw_log/log.h"

namespace bms::threads {

AcquisitionThreadCore::AcquisitionThreadCore(
    bms::domain::RawInputsSink& raw_inputs_sink)
    : raw_inputs_sink_(raw_inputs_sink),
      sensor_(*this),
      current_timer_(
          [this](pw::chrono::SystemClock::time_point t) { OnCurrentTimer(t); }),
      voltage_timer_(
          [this](pw::chrono::SystemClock::time_point t) { OnVoltageTimer(t); }),
      temperature_timer_([this](pw::chrono::SystemClock::time_point t) {
        OnTemperatureTimer(t);
      }) {}

uint32_t AcquisitionThreadCore::NowMs() const {
  const auto elapsed = pw::chrono::SystemClock::now() - start_time_;

  return static_cast<uint32_t>(
      std::chrono::duration_cast<std::chrono::milliseconds>(elapsed).count());
}

void AcquisitionThreadCore::OnCurrentTimer(
    pw::chrono::SystemClock::time_point) {
  current_timer_.InvokeAfter(kCurrentPeriod);
  sensor_.RequestCurrent(NowMs());
}

void AcquisitionThreadCore::OnVoltageTimer(
    pw::chrono::SystemClock::time_point) {
  voltage_timer_.InvokeAfter(kVoltagePeriod);
  sensor_.RequestVoltage(NowMs());
}

void AcquisitionThreadCore::OnTemperatureTimer(
    pw::chrono::SystemClock::time_point) {
  temperature_timer_.InvokeAfter(kTemperaturePeriod);
  sensor_.RequestTemperature(NowMs());
}

void AcquisitionThreadCore::OnSensorDataReady(
    const bms::abstraction::SensorSample& sample) {
  PushSensorSample(sample);
}

void AcquisitionThreadCore::PushSensorSample(
    const bms::abstraction::SensorSample& sample) {
  {
    std::lock_guard<pw::sync::Mutex> lock(mutex_);

    if (sensor_queue_count_ >= kSensorQueueSize) {
      sensor_queue_tail_ = (sensor_queue_tail_ + 1U) % kSensorQueueSize;
      --sensor_queue_count_;

      PW_LOG_WARN("Sensor queue full, dropping oldest sample");
    }

    sensor_queue_[sensor_queue_head_] = sample;
    sensor_queue_head_ = (sensor_queue_head_ + 1U) % kSensorQueueSize;
    ++sensor_queue_count_;
  }

  sensor_queue_notification_.release();
}

bool AcquisitionThreadCore::PopSensorSample(
    bms::abstraction::SensorSample& sample) {
  std::lock_guard<pw::sync::Mutex> lock(mutex_);

  if (sensor_queue_count_ == 0U) {
    return false;
  }

  sample = sensor_queue_[sensor_queue_tail_];
  sensor_queue_tail_ = (sensor_queue_tail_ + 1U) % kSensorQueueSize;
  --sensor_queue_count_;

  return true;
}

void AcquisitionThreadCore::ApplySensorSample(
    const bms::abstraction::SensorSample& sample) {
  std::lock_guard<pw::sync::Mutex> lock(mutex_);

  switch (sample.channel) {
    case bms::abstraction::SensorChannel::kVoltage:
      raw_inputs_.pack_voltage_mv.value =
          sample.value < 0 ? 0U : static_cast<uint32_t>(sample.value);
      raw_inputs_.pack_voltage_mv.tick_ms = sample.tick_ms;
      raw_inputs_.pack_voltage_mv.valid = sample.valid && sample.value >= 0;
      break;

    case bms::abstraction::SensorChannel::kCurrent:
      raw_inputs_.pack_current_ma.value = sample.value;
      raw_inputs_.pack_current_ma.tick_ms = sample.tick_ms;
      raw_inputs_.pack_current_ma.valid = sample.valid;
      break;

    case bms::abstraction::SensorChannel::kTemperature:
      raw_inputs_.pack_temperature_mc.value = sample.value;
      raw_inputs_.pack_temperature_mc.tick_ms = sample.tick_ms;
      raw_inputs_.pack_temperature_mc.valid = sample.valid;
      break;
  }
}

void AcquisitionThreadCore::PublishRawInputs() {
  bms::domain::RawInputs msg{};

  {
    std::lock_guard<pw::sync::Mutex> lock(mutex_);
    msg = raw_inputs_;
  }

  raw_inputs_sink_.PostRawInputs(msg);
}

void AcquisitionThreadCore::Run() {
  PW_LOG_INFO("Acquisition thread started");

  start_time_ = pw::chrono::SystemClock::now();

  current_timer_.InvokeAfter(kCurrentPeriod);
  voltage_timer_.InvokeAfter(kVoltagePeriod);
  temperature_timer_.InvokeAfter(kTemperaturePeriod);

  while (true) {
    sensor_queue_notification_.acquire();

    bms::abstraction::SensorSample sample{};

    while (PopSensorSample(sample)) {
      ApplySensorSample(sample);
      PublishRawInputs();
    }
  }
}

}  // namespace bms::threads
