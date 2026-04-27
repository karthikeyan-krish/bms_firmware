#pragma once

#include <cstdint>

namespace bms::domain {

struct ProcessedInputs {
  uint32_t pack_voltage_mv{0U};
  int32_t pack_current_ma{0};
  int32_t pack_temperature_mc{0};

  bool voltage_valid{false};
  bool current_valid{false};
  bool temperature_valid{false};
};

}  // namespace bms::domain
