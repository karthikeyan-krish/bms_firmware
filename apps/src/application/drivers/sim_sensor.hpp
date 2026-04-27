#pragma once

#include <cstdint>

namespace bms::drivers {

enum class SensorFault { kOk = 0, kDisconnected, kStuck, kNoisy, kSpike };

class SimSensor {
 public:
  SimSensor(int32_t base_value,
            uint32_t noise_amplitude,
            int32_t min_value,
            int32_t max_value);

  virtual ~SimSensor() = default;

  void SetFault(SensorFault fault);
  void SetBaseValue(int32_t value);
  void SetStuckValue(int32_t value);
  void SetSpikeValue(int32_t value);

  bool Read(int32_t& out_value);

 protected:
  virtual int32_t GenerateNominalValue();

  int32_t Clamp(int32_t value) const;
  int32_t GenerateNoise();

  int32_t base_value_;
  uint32_t noise_amplitude_;
  int32_t min_value_;
  int32_t max_value_;
  int32_t stuck_value_;
  int32_t spike_value_;
  SensorFault fault_;
  uint32_t tick_;
  int32_t last_value_;
};

class VoltageSensor : public SimSensor {
 public:
  VoltageSensor();

 protected:
  int32_t GenerateNominalValue() override;
};

class CurrentSensor : public SimSensor {
 public:
  CurrentSensor();

 protected:
  int32_t GenerateNominalValue() override;
};

class TemperatureSensor : public SimSensor {
 public:
  TemperatureSensor();

 protected:
  int32_t GenerateNominalValue() override;
};

}  // namespace bms::drivers
