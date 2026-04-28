#pragma once

#include <cstdint>
// #include <optional>

namespace bms::domain {

struct TimestampedValueU32 {
  uint32_t value{0U};
  uint32_t tick_ms{0U};
  bool valid{false};
};

struct TimestampedValueI32 {
  int32_t value{0};
  uint32_t tick_ms{0U};
  bool valid{false};
};

struct RawInputs {
  TimestampedValueU32 pack_voltage_mv;
  TimestampedValueI32 pack_current_ma;
  TimestampedValueI32 pack_temperature_mc;
  bool voltage_new{false};
  bool current_new{false};
  bool temperature_new{false};
};

}  // namespace bms::domain
