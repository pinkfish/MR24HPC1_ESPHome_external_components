#include "restart_button.h"

namespace esphome {
namespace seeed_mr24hpc1pf {

void RestartButton::press_action() { this->parent_->set_restart(); }

}  // namespace seeed_mr24hpc1pf
}  // namespace esphome
