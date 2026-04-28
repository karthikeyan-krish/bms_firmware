#pragma once

#include <cstdint>

namespace bms::domain::parameters {

// Acquisition queue sizing and periods.
inline constexpr uint32_t kAcquisitionSensorQueueSize = 16U;
inline constexpr uint32_t kCurrentSamplePeriodMs = 10U;
inline constexpr uint32_t kVoltageSamplePeriodMs = 20U;
inline constexpr uint32_t kTemperatureSamplePeriodMs = 100U;

// Simulated sensor ranges and nominal values.
inline constexpr int32_t kVoltageNominalMv = 35000;
inline constexpr uint32_t kVoltageNoiseAmplitudeMv = 200U;
inline constexpr int32_t kVoltageMinMv = 30000;
inline constexpr int32_t kVoltageMaxMv = 45000;
inline constexpr int32_t kVoltageSpikeMv = 58000;

inline constexpr int32_t kCurrentNominalMa = 100000;
inline constexpr uint32_t kCurrentNoiseAmplitudeMa = 500U;
inline constexpr int32_t kCurrentMinMa = -50000;
inline constexpr int32_t kCurrentMaxMa = 150000;
inline constexpr int32_t kCurrentSpikeMa = 180000;

inline constexpr int32_t kTemperatureNominalMc = 25000;
inline constexpr uint32_t kTemperatureNoiseAmplitudeMc = 300U;
inline constexpr int32_t kTemperatureMinMc = -5000;
inline constexpr int32_t kTemperatureMaxMc = 65000;
inline constexpr int32_t kTemperatureSpikeMc = 75000;

// Processing queue sizing.
inline constexpr uint32_t kProcessingQueueDepth = 16U;

// Safety limits.
inline constexpr uint32_t kOvervoltageLimitMv = 40000U;
inline constexpr uint32_t kOvervoltageRecoveryMv = 35000U;
inline constexpr uint32_t kUndervoltageLimitMv = 29000U;
inline constexpr uint32_t kUndervoltageRecoveryMv = 30000U;
inline constexpr int32_t kOvertemperatureLimitMc = 60000;
inline constexpr int32_t kOvertemperatureRecoveryMc = 55000;
inline constexpr int32_t kUndertemperatureLimitMc = -20000;
inline constexpr int32_t kUndertemperatureRecoveryMc = -15000;

// Current stress model.
inline constexpr uint32_t kStressScale = 1000000U;

inline constexpr uint32_t kCurrentLowThresholdMa = 100000U;
inline constexpr uint32_t kCurrentMediumThresholdMa = 120000U;
inline constexpr uint32_t kCurrentHighThresholdMa = 150000U;
inline constexpr uint32_t kCurrentCutoffThresholdMa = 170000U;

inline constexpr uint32_t kCurrentLowAllowedTimeMs = 20000U;
inline constexpr uint32_t kCurrentMediumAllowedTimeMs = 10000U;
inline constexpr uint32_t kCurrentHighAllowedTimeMs = 5000U;
inline constexpr uint32_t kCurrentRecoveryThresholdMa = 90000U;
inline constexpr uint32_t kCurrentRecoveryDelayMs = 1000U;

}  // namespace bms::domain::parameters
