#pragma once

namespace bms::domain {

struct FaultStatus {
  bool sensor_fault{false};
  bool overvoltage{false};
  bool overcurrent{false};
  bool overtemperature{false};
  bool undertemperature{false};
};

}  // namespace bms::domain

