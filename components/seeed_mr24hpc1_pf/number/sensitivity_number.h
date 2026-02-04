#pragma once

#include "esphome/components/number/number.h"
#include "../seeed_mr24hpc1_pf.h"

namespace esphome {
namespace seeed_mr24hpc1_pf {

class SensitivityNumber : public number::Number, public Parented<MR24HPC1PFComponent> {
 public:
  SensitivityNumber() = default;

 protected:
  void control(float value) override;
};

}  // namespace seeed_mr24hpc1_pf
}  // namespace esphome
