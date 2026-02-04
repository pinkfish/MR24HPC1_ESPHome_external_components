#include "motion_timeout_select.h"

namespace esphome {
namespace seeed_mr24hpc1pf {

void MotionTimeoutSelect::control(const std::string &value) {
  this->publish_state(value);
  auto index = this->index_of(value);
  if (index.has_value()) {
    this->parent_->set_unman_time(index.value());
  }
}

}  // namespace seeed_mr24hpc1pf
}  // namespace esphome
