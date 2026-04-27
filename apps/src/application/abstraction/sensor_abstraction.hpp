#pragma once

#include <cstdint>

#include "sim_sensor.hpp"

namespace bms::abstraction {

enum class SensorChannel {
  kVoltage,
  kCurrent,
  kTemperature,
};

struct SensorSample {
  SensorChannel channel;
  int32_t value;
  uint32_t tick_ms;
  bool valid;
};

class BmsSensorAbstraction {
 public:
  class Callback {
   public:
    virtual ~Callback() = default;
    virtual void OnSensorDataReady(const SensorSample& sample) = 0;
  };

  explicit BmsSensorAbstraction(Callback& callback) : callback_(callback) {}

  void RequestVoltage(uint32_t tick_ms) {
    RequestSensor(SensorChannel::kVoltage, voltage_sensor_, tick_ms);
  }

  void RequestCurrent(uint32_t tick_ms) {
    RequestSensor(SensorChannel::kCurrent, current_sensor_, tick_ms);
  }

  void RequestTemperature(uint32_t tick_ms) {
    RequestSensor(SensorChannel::kTemperature, temperature_sensor_, tick_ms);
  }

 private:
  void RequestSensor(SensorChannel channel,
                     bms::drivers::SimSensor& sensor,
                     uint32_t tick_ms) {
    SensorSample sample{};
    sample.channel = channel;
    sample.tick_ms = tick_ms;
    sample.valid = sensor.Read(sample.value);

    callback_.OnSensorDataReady(sample);
  }

  Callback& callback_;

  bms::drivers::VoltageSensor voltage_sensor_;
  bms::drivers::CurrentSensor current_sensor_;
  bms::drivers::TemperatureSensor temperature_sensor_;
};

}  // namespace bms::abstraction
