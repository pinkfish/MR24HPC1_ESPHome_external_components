#pragma once

#include "esphome/components/select/select.h"
#include "../seeed_mr24hpc1.h"

namespace esphome {
namespace seeed_mr24hpc1 {

class MotionTimeoutSelect : public select::Select, public Parented<MR24HPC1Component> {
 public:
  MotionTimeoutSelect() = default;

 protected:
  void control(const std::string &value) override;
};

}  // namespace seeed_mr24hpc1
}  // namespace esphome
