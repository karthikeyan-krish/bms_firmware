#pragma once

#include "raw_inputs.hpp"

namespace bms::domain {

class RawInputsSink {
 public:
  virtual ~RawInputsSink() = default;
  virtual bool PostRawInputs(const RawInputs& raw_inputs) = 0;
};

}  // namespace bms::domain
