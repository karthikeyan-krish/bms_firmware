#include <gtest/gtest.h>

#include <cstdint>

#include "pw_chrono/system_clock.h"
#include "pw_chrono/system_timer.h"
#include "pw_sync/mutex.h"
#include "pw_sync/thread_notification.h"
#include "pw_thread/thread_core.h"

#define private public
#include "acquisition_thread.hpp"
#undef private

namespace {

class RecordingRawInputsSink final : public bms::domain::RawInputsSink {
 public:
  void PostRawInputs(const bms::domain::RawInputs& raw_inputs) override {
    latest = raw_inputs;
    ++post_count;
  }

  bms::domain::RawInputs latest{};
  uint32_t post_count{0U};
};

bms::abstraction::SensorSample MakeSample(
    bms::abstraction::SensorChannel channel,
    int32_t value,
    uint32_t tick_ms,
    bool valid = true) {
  return {channel, value, tick_ms, valid};
}

TEST(AcquisitionThreadCoreTest, QueuesSensorSamplesAndDropsOldestOnOverflow) {
  RecordingRawInputsSink sink;
  bms::threads::AcquisitionThreadCore acquisition(sink);

  for (uint32_t i = 0U; i < 17U; ++i) {
    acquisition.OnSensorDataReady(
        MakeSample(bms::abstraction::SensorChannel::kVoltage, 30000U + i, i));
  }

  for (uint32_t expected = 1U; expected <= 16U; ++expected) {
    bms::abstraction::SensorSample sample{};
    ASSERT_TRUE(acquisition.PopSensorSample(sample));
    EXPECT_EQ(sample.value, static_cast<int32_t>(30000U + expected));
    EXPECT_EQ(sample.tick_ms, expected);
  }

  bms::abstraction::SensorSample sample{};
  EXPECT_FALSE(acquisition.PopSensorSample(sample));
}

TEST(AcquisitionThreadCoreTest, ApplySensorSampleUpdatesMatchingRawField) {
  RecordingRawInputsSink sink;
  bms::threads::AcquisitionThreadCore acquisition(sink);

  acquisition.ApplySensorSample(
      MakeSample(bms::abstraction::SensorChannel::kVoltage, 48000U, 10U));
  acquisition.ApplySensorSample(
      MakeSample(bms::abstraction::SensorChannel::kCurrent, 12000U, 20U));
  acquisition.ApplySensorSample(MakeSample(
      bms::abstraction::SensorChannel::kTemperature, 25000U, 30U, false));

  EXPECT_EQ(acquisition.raw_inputs_.pack_voltage_mv.value, 48000U);
  EXPECT_EQ(acquisition.raw_inputs_.pack_voltage_mv.tick_ms, 10U);
  EXPECT_TRUE(acquisition.raw_inputs_.pack_voltage_mv.valid);

  EXPECT_EQ(acquisition.raw_inputs_.pack_current_ma.value, 12000);
  EXPECT_EQ(acquisition.raw_inputs_.pack_current_ma.tick_ms, 20U);
  EXPECT_TRUE(acquisition.raw_inputs_.pack_current_ma.valid);

  EXPECT_EQ(acquisition.raw_inputs_.pack_temperature_mc.value, 25000);
  EXPECT_EQ(acquisition.raw_inputs_.pack_temperature_mc.tick_ms, 30U);
  EXPECT_FALSE(acquisition.raw_inputs_.pack_temperature_mc.valid);
}

TEST(AcquisitionThreadCoreTest, PublishRawInputsPostsToSink) {
  RecordingRawInputsSink sink;
  bms::threads::AcquisitionThreadCore acquisition(sink);

  acquisition.ApplySensorSample(
      MakeSample(bms::abstraction::SensorChannel::kVoltage, 48000U, 10U));
  acquisition.ApplySensorSample(
      MakeSample(bms::abstraction::SensorChannel::kCurrent, 12000U, 20U));
  acquisition.ApplySensorSample(
      MakeSample(bms::abstraction::SensorChannel::kTemperature, 25000U, 30U));

  acquisition.PublishRawInputs();

  EXPECT_EQ(sink.post_count, 1U);
  EXPECT_EQ(sink.latest.pack_voltage_mv.value, 48000U);
  EXPECT_EQ(sink.latest.pack_current_ma.value, 12000);
  EXPECT_EQ(sink.latest.pack_temperature_mc.value, 25000);
}

TEST(AcquisitionThreadCoreTest, CurrentAndTemperaturePreserveNegativeSamples) {
  RecordingRawInputsSink sink;
  bms::threads::AcquisitionThreadCore acquisition(sink);

  acquisition.ApplySensorSample(
      MakeSample(bms::abstraction::SensorChannel::kCurrent, -12000, 20U));
  acquisition.ApplySensorSample(
      MakeSample(bms::abstraction::SensorChannel::kTemperature, -5000, 30U));

  EXPECT_EQ(acquisition.raw_inputs_.pack_current_ma.value, -12000);
  EXPECT_TRUE(acquisition.raw_inputs_.pack_current_ma.valid);

  EXPECT_EQ(acquisition.raw_inputs_.pack_temperature_mc.value, -5000);
  EXPECT_TRUE(acquisition.raw_inputs_.pack_temperature_mc.valid);
}

TEST(AcquisitionThreadCoreTest, NegativeVoltageSampleIsMarkedInvalid) {
  RecordingRawInputsSink sink;
  bms::threads::AcquisitionThreadCore acquisition(sink);

  acquisition.ApplySensorSample(
      MakeSample(bms::abstraction::SensorChannel::kVoltage, -1, 10U));

  EXPECT_EQ(acquisition.raw_inputs_.pack_voltage_mv.value, 0U);
  EXPECT_FALSE(acquisition.raw_inputs_.pack_voltage_mv.valid);
}

}  // namespace
