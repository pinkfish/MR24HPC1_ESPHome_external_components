#pragma once

#include "esphome/components/button/button.h"
#include "../seeed_mr24hpc1pf.h"

namespace esphome {
namespace seeed_mr24hpc1pf {

class RestartButton : public button::Button, public Parented<MR24HPC1PFComponent> {
 public:
  RestartButton() = default;

 protected:
  void press_action() override;
};

}  // namespace seeed_mr24hpc1pf
}  // namespace esphome
