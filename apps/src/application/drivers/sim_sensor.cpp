#include "sim_sensor.hpp"

#include "bms_parameters.hpp"

namespace bms::drivers {

SimSensor::SimSensor(int32_t base_value,
                     uint32_t noise_amplitude,
                     int32_t min_value,
                     int32_t max_value)
    : base_value_(base_value),
      noise_amplitude_(noise_amplitude),
      min_value_(min_value),
      max_value_(max_value),
      stuck_value_(base_value),
      spike_value_(max_value),
      fault_(SensorFault::kOk),
      tick_(0U),
      last_value_(base_value) {}

void SimSensor::SetFault(SensorFault fault) { fault_ = fault; }

void SimSensor::SetBaseValue(int32_t value) { base_value_ = value; }

void SimSensor::SetStuckValue(int32_t value) { stuck_value_ = value; }

void SimSensor::SetSpikeValue(int32_t value) { spike_value_ = value; }

int32_t SimSensor::Clamp(int32_t value) const {
  if (value < min_value_) {
    return min_value_;
  }

  if (value > max_value_) {
    return max_value_;
  }

  return value;
}

int32_t SimSensor::GenerateNoise() {
  const int32_t pattern = static_cast<int32_t>((tick_ * 17U) % 11U) - 5;

  return (pattern * static_cast<int32_t>(noise_amplitude_)) / 5;
}

int32_t SimSensor::GenerateNominalValue() {
  return base_value_ + GenerateNoise();
}

bool SimSensor::Read(int32_t& out_value) {
  ++tick_;

  int32_t value = base_value_;

  switch (fault_) {
    case SensorFault::kDisconnected:
      return false;

    case SensorFault::kStuck:
      value = stuck_value_;
      break;

    case SensorFault::kNoisy:
      value = base_value_ + (GenerateNoise() * 3);
      break;

    case SensorFault::kSpike:
      if ((tick_ % 20U) == 0U) {
        value = spike_value_;
      } else {
        value = GenerateNominalValue();
      }
      break;

    case SensorFault::kOk:
    default:
      value = GenerateNominalValue();
      break;
  }

  value = Clamp(value);
  last_value_ = value;
  out_value = value;
  return true;
}

VoltageSensor::VoltageSensor()
    : SimSensor(bms::domain::parameters::kVoltageNominalMv,
                bms::domain::parameters::kVoltageNoiseAmplitudeMv,
                bms::domain::parameters::kVoltageMinMv,
                bms::domain::parameters::kVoltageMaxMv) {
  SetSpikeValue(bms::domain::parameters::kVoltageSpikeMv);
}

int32_t VoltageSensor::GenerateNominalValue() {
  return SimSensor::GenerateNominalValue();
}

CurrentSensor::CurrentSensor()
    : SimSensor(bms::domain::parameters::kCurrentNominalMa,
                bms::domain::parameters::kCurrentNoiseAmplitudeMa,
                bms::domain::parameters::kCurrentMinMa,
                bms::domain::parameters::kCurrentMaxMa) {
  SetSpikeValue(bms::domain::parameters::kCurrentSpikeMa);
}

int32_t CurrentSensor::GenerateNominalValue() {
  return SimSensor::GenerateNominalValue();
}

TemperatureSensor::TemperatureSensor()
    : SimSensor(bms::domain::parameters::kTemperatureNominalMc,
                bms::domain::parameters::kTemperatureNoiseAmplitudeMc,
                bms::domain::parameters::kTemperatureMinMc,
                bms::domain::parameters::kTemperatureMaxMc) {
  SetSpikeValue(bms::domain::parameters::kTemperatureSpikeMc);
}

int32_t TemperatureSensor::GenerateNominalValue() {
  return SimSensor::GenerateNominalValue();
}

}  // namespace bms::drivers
