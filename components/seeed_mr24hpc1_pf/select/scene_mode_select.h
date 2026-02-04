#pragma once

#include "esphome/components/select/select.h"
#include "../seeed_mr24hpc1pf.h"

namespace esphome {
namespace seeed_mr24hpc1pf {

class SceneModeSelect : public select::Select, public Parented<MR24HPC1PFComponent> {
 public:
  SceneModeSelect() = default;

 protected:
  void control(const std::string &value) override;
};

}  // namespace seeed_mr24hpc1pf
}  // namespace esphome
