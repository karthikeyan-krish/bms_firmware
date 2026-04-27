#include <gtest/gtest.h>

#include <cstdint>

#include "pw_sync/mutex.h"
#include "pw_sync/thread_notification.h"
#include "pw_thread/thread_core.h"

#define private public
#include "processing_thread.hpp"
#undef private

namespace {

bms::domain::RawInputs MakeRaw(uint32_t voltage_mv,
                               int32_t current_ma,
                               int32_t temperature_mc) {
  bms::domain::RawInputs raw{};
  raw.pack_voltage_mv = {voltage_mv, 1U, true};
  raw.pack_current_ma = {current_ma, 1U, true};
  raw.pack_temperature_mc = {temperature_mc, 1U, true};
  return raw;
}

TEST(ProcessingThreadCoreTest, QueueOverflowDropsOldestRawInput) {
  bms::threads::ProcessingThreadCore processing;

  for (uint32_t i = 0U; i < 17U; ++i) {
    bms::domain::RawInputs raw = MakeRaw(30000U + i, 1000U, 25000U);
    ASSERT_TRUE(processing.PostRawInputs(raw));
  }

  for (uint32_t expected = 1U; expected <= 16U; ++expected) {
    bms::domain::RawInputs raw{};
    ASSERT_TRUE(processing.PopRawInputs(raw));
    EXPECT_EQ(raw.pack_voltage_mv.value, 30000U + expected);
  }

  bms::domain::RawInputs raw{};
  EXPECT_FALSE(processing.PopRawInputs(raw));
}

TEST(ProcessingThreadCoreTest, ProcessSampleUpdatesProcessedInputs) {
  bms::threads::ProcessingThreadCore processing;

  processing.ProcessSample(MakeRaw(36000U, 12000U, 26000U));

  const bms::domain::ProcessedInputs processed =
      processing.GetLatestProcessedInputs();
  EXPECT_EQ(processed.pack_voltage_mv, 36000U);
  EXPECT_EQ(processed.pack_current_ma, 12000);
  EXPECT_EQ(processed.pack_temperature_mc, 26000);
  EXPECT_TRUE(processed.voltage_valid);
  EXPECT_TRUE(processed.current_valid);
  EXPECT_TRUE(processed.temperature_valid);

  const bms::domain::FaultStatus faults = processing.GetLatestFaultStatus();
  EXPECT_FALSE(faults.sensor_fault);
  EXPECT_FALSE(faults.overvoltage);
  EXPECT_FALSE(faults.overcurrent);
  EXPECT_FALSE(faults.overtemperature);
  EXPECT_FALSE(faults.undertemperature);
}

TEST(ProcessingThreadCoreTest, InvalidChannelsSetSensorFault) {
  bms::threads::ProcessingThreadCore processing;

  bms::domain::RawInputs raw{};
  raw.pack_voltage_mv = {36000U, 1U, true};

  processing.ProcessSample(raw);

  const bms::domain::FaultStatus faults = processing.GetLatestFaultStatus();
  EXPECT_TRUE(faults.sensor_fault);
}

TEST(ProcessingThreadCoreTest, VoltageAndTemperatureThresholdsSetFaults) {
  bms::threads::ProcessingThreadCore processing;

  processing.ProcessSample(MakeRaw(41000U, 1000U, 61000U));

  const bms::domain::FaultStatus faults = processing.GetLatestFaultStatus();
  EXPECT_TRUE(faults.overvoltage);
  EXPECT_FALSE(faults.overcurrent);
  EXPECT_TRUE(faults.overtemperature);
  EXPECT_FALSE(faults.undertemperature);
}

TEST(ProcessingThreadCoreTest, NegativeTemperatureBelowLimitSetsFault) {
  bms::threads::ProcessingThreadCore processing;

  processing.ProcessSample(MakeRaw(36000U, 1000, -21000));

  const bms::domain::FaultStatus faults = processing.GetLatestFaultStatus();
  EXPECT_FALSE(faults.overtemperature);
  EXPECT_TRUE(faults.undertemperature);
}

TEST(ProcessingThreadCoreTest, CurrentCutoffTripsImmediatelyAndRecovers) {
  bms::threads::ProcessingThreadCore processing;

  EXPECT_TRUE(processing.DetectCurrentFault(170000));
  EXPECT_TRUE(processing.current_fault_active_);
  EXPECT_EQ(processing.current_stress_, processing.kStressScale);

  EXPECT_FALSE(processing.DetectCurrentFault(89000));
  EXPECT_FALSE(processing.current_fault_active_);
  EXPECT_EQ(processing.current_stress_, 0U);
}

TEST(ProcessingThreadCoreTest, NegativeCurrentUsesAbsoluteValueForFaults) {
  bms::threads::ProcessingThreadCore processing;

  EXPECT_TRUE(processing.DetectCurrentFault(-170000));
  EXPECT_TRUE(processing.current_fault_active_);
  EXPECT_EQ(processing.current_stress_, processing.kStressScale);
}

TEST(ProcessingThreadCoreTest, HighCurrentAccumulatesStressOverTime) {
  bms::threads::ProcessingThreadCore processing;

  for (uint32_t i = 0U; i < 499U; ++i) {
    EXPECT_FALSE(processing.DetectCurrentFault(150000));
  }

  EXPECT_TRUE(processing.DetectCurrentFault(150000));
}

}  // namespace
