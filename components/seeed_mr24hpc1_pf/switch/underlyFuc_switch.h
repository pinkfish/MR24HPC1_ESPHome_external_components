#pragma once

#include "esphome/components/switch/switch.h"
#include "../seeed_mr24hpc1_pf.h"

namespace esphome {
namespace seeed_mr24hpc1_pf {

class UnderlyOpenFunctionSwitch : public switch_::Switch, public Parented<MR24HPC1PFComponent> {
 public:
  UnderlyOpenFunctionSwitch() = default;

 protected:
  void write_state(bool state) override;
};

}  // namespace seeed_mr24hpc1_pf
}  // namespace esphome
