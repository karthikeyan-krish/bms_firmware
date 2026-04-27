#pragma once

#include <cstdint>

namespace bms::domain::parameters {

// Simulated sensor ranges and nominal values.
inline constexpr int32_t kVoltageNominalMv = 35000;
inline constexpr uint32_t kVoltageNoiseAmplitudeMv = 200U;
inline constexpr int32_t kVoltageMinMv = 30000;
inline constexpr int32_t kVoltageMaxMv = 45000;
inline constexpr int32_t kVoltageSpikeMv = 58000;

inline constexpr int32_t kCurrentNominalMa = 10000;
inline constexpr uint32_t kCurrentNoiseAmplitudeMa = 500U;
inline constexpr int32_t kCurrentMinMa = -50000;
inline constexpr int32_t kCurrentMaxMa = 100000;
inline constexpr int32_t kCurrentSpikeMa = 80000;

inline constexpr int32_t kTemperatureNominalMc = 25000;
inline constexpr uint32_t kTemperatureNoiseAmplitudeMc = 300U;
inline constexpr int32_t kTemperatureMinMc = -5000;
inline constexpr int32_t kTemperatureMaxMc = 65000;
inline constexpr int32_t kTemperatureSpikeMc = 75000;

}  // namespace bms::domain::parameters
