#pragma once

#include "esphome/components/button/button.h"
#include "../seeed_mr24hpc1_pf.h"

namespace esphome {
namespace seeed_mr24hpc1_pf {

class RestartButton : public button::Button, public Parented<MR24HPC1PFComponent> {
 public:
  RestartButton() = default;

 protected:
  void press_action() override;
};

}  // namespace seeed_mr24hpc1_pf
}  // namespace esphome
