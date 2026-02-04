#pragma once

#include "esphome/components/select/select.h"
#include "../seeed_mr24hpc1_pf.h"

namespace esphome {
namespace seeed_mr24hpc1_pf {

class MotionTimeoutSelect : public select::Select, public Parented<MR24HPC1PFComponent> {
 public:
  MotionTimeoutSelect() = default;

 protected:
  void control(const std::string &value) override;
};

}  // namespace seeed_mr24hpc1_pf
}  // namespace esphome
