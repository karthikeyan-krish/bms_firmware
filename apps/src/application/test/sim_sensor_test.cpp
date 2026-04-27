#include "sim_sensor.hpp"

#include <gtest/gtest.h>

#include <cstdint>

namespace {

TEST(SimSensorTest, NominalReadAppliesDeterministicSignedNoise) {
  bms::drivers::SimSensor sensor(100U, 10U, 0U, 200U);

  int32_t value = 0;
  ASSERT_TRUE(sensor.Read(value));
  EXPECT_EQ(value, 102);

  ASSERT_TRUE(sensor.Read(value));
  EXPECT_EQ(value, 92);
}

TEST(SimSensorTest, DisconnectedReadFails) {
  bms::drivers::SimSensor sensor(100U, 10U, 0U, 200U);
  sensor.SetFault(bms::drivers::SensorFault::kDisconnected);

  int32_t value = 123;
  EXPECT_FALSE(sensor.Read(value));
  EXPECT_EQ(value, 123);
}

TEST(SimSensorTest, StuckFaultReturnsConfiguredValue) {
  bms::drivers::SimSensor sensor(100U, 10U, 0U, 200U);
  sensor.SetFault(bms::drivers::SensorFault::kStuck);
  sensor.SetStuckValue(77U);

  int32_t value = 0;
  ASSERT_TRUE(sensor.Read(value));
  EXPECT_EQ(value, 77);
}

TEST(SimSensorTest, SpikeFaultUsesSpikeValueEveryTwentiethRead) {
  bms::drivers::SimSensor sensor(100U, 10U, 0U, 200U);
  sensor.SetFault(bms::drivers::SensorFault::kSpike);
  sensor.SetSpikeValue(180U);

  int32_t value = 0;
  for (uint32_t i = 1U; i < 20U; ++i) {
    ASSERT_TRUE(sensor.Read(value));
    EXPECT_NE(value, 180);
  }

  ASSERT_TRUE(sensor.Read(value));
  EXPECT_EQ(value, 180);
}

TEST(SimSensorTest, ValuesAreClampedToConfiguredRange) {
  bms::drivers::SimSensor sensor(100U, 10U, 50U, 150U);
  sensor.SetFault(bms::drivers::SensorFault::kStuck);

  int32_t value = 0;

  sensor.SetStuckValue(10U);
  ASSERT_TRUE(sensor.Read(value));
  EXPECT_EQ(value, 50);

  sensor.SetStuckValue(200U);
  ASSERT_TRUE(sensor.Read(value));
  EXPECT_EQ(value, 150);
}

TEST(SimSensorTest, SignedSensorCanReportNegativeValues) {
  bms::drivers::SimSensor sensor(-100, 10U, -200, 200);

  int32_t value = 0;
  ASSERT_TRUE(sensor.Read(value));
  EXPECT_EQ(value, -98);

  ASSERT_TRUE(sensor.Read(value));
  EXPECT_EQ(value, -108);
}

}  // namespace
