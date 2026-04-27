#include "sensor_abstraction.hpp"

#include <gtest/gtest.h>

#include <cstdint>
#include <vector>

namespace {

class RecordingCallback final
    : public bms::abstraction::BmsSensorAbstraction::Callback {
 public:
  void OnSensorDataReady(
      const bms::abstraction::SensorSample& sample) override {
    samples.push_back(sample);
  }

  std::vector<bms::abstraction::SensorSample> samples;
};

TEST(BmsSensorAbstractionTest, RequestsReportExpectedChannelsAndTicks) {
  RecordingCallback callback;
  bms::abstraction::BmsSensorAbstraction sensor(callback);

  sensor.RequestVoltage(10U);
  sensor.RequestCurrent(20U);
  sensor.RequestTemperature(30U);

  ASSERT_EQ(callback.samples.size(), 3U);

  EXPECT_EQ(callback.samples[0].channel,
            bms::abstraction::SensorChannel::kVoltage);
  EXPECT_EQ(callback.samples[0].tick_ms, 10U);
  EXPECT_TRUE(callback.samples[0].valid);

  EXPECT_EQ(callback.samples[1].channel,
            bms::abstraction::SensorChannel::kCurrent);
  EXPECT_EQ(callback.samples[1].tick_ms, 20U);
  EXPECT_TRUE(callback.samples[1].valid);

  EXPECT_EQ(callback.samples[2].channel,
            bms::abstraction::SensorChannel::kTemperature);
  EXPECT_EQ(callback.samples[2].tick_ms, 30U);
  EXPECT_TRUE(callback.samples[2].valid);
}

}  // namespace
