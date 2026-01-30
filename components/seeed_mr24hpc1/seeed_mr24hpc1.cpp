#include "seeed_mr24hpc1.h"

#include "esphome/core/log.h"

#include <utility>

namespace esphome {
namespace seeed_mr24hpc1 {

static const char *const TAG = "seeed_mr24hpc1";

// Prints the component's configuration data. dump_config() prints all of the component's configuration
// items in an easy-to-read format, including the configuration key-value pairs.
void MR24HPC1Component::dump_config() {
  ESP_LOGCONFIG(TAG, "MR24HPC1:");
#ifdef USE_TEXT_SENSOR
  LOG_TEXT_SENSOR(" ", "Heartbeat Text Sensor", this->heartbeat_state_text_sensor_);
  LOG_TEXT_SENSOR(" ", "Product Model Text Sensor", this->product_model_text_sensor_);
  LOG_TEXT_SENSOR(" ", "Product ID Text Sensor", this->product_id_text_sensor_);
  LOG_TEXT_SENSOR(" ", "Hardware Model Text Sensor", this->hardware_model_text_sensor_);
  LOG_TEXT_SENSOR(" ", "Firware Verison Text Sensor", this->firware_version_text_sensor_);
  LOG_TEXT_SENSOR(" ", "Keep Away Text Sensor", this->keep_away_text_sensor_);
  LOG_TEXT_SENSOR(" ", "Motion Status Text Sensor", this->motion_status_text_sensor_);
#endif
#ifdef USE_BINARY_SENSOR
  LOG_BINARY_SENSOR(" ", "Has Target Binary Sensor", this->has_target_binary_sensor_);
#endif
#ifdef USE_SENSOR
  LOG_SENSOR(" ", "Movement Signs Sensor", this->movement_signs_sensor_);
  LOG_SENSOR(" ", "Existance Energy Sensor", this->existence_energy_sensor_);
  LOG_SENSOR(" ", "Motion Energy Sensor", this->motion_energy_sensor_);
  LOG_SENSOR(" ", "Motion Speed Sensor", this->motion_speed_sensor_);
  LOG_SENSOR(" ", "Static Distance Sensor", this->static_distance_sensor_);
  LOG_SENSOR(" ", "Motion Distance Sensor", this->motion_distance_sensor_);
#endif
#ifdef USE_BUTTON
  LOG_BUTTON(" ", "Restart Button", this->restart_button_);
#endif
#ifdef USE_SELECT
  LOG_SELECT(" ", "Presence Timeout Select", this->motion_timeout_select);
  LOG_SELECT(" ", "Scene Mode Select", this->scene_mode_select_);
#endif
#ifdef USE_NUMBER
  LOG_NUMBER(" ", "Sensitivity Number", this->sensitivity_number_);
  LOG_NUMBER(" ", "Existence Threshold Number", this->existence_threshold_number_);
  LOG_NUMBER(" ", "Motion Threshold Number", this->motion_threshold_number_);
  LOG_NUMBER(" ", "Motion Trigger Time Number", this->motion_trigger_number_);
  LOG_NUMBER(" ", "Motion To Rest Time Number", this->motion_to_rest_number_);
#endif
}

// Initialisation functions
void MR24HPC1Component::setup() {
  ESP_LOGCONFIG(TAG, "Setting up MR24HPC1...");
  this->check_uart_settings(115200);

  this->last_recv_time_ = millis();
  this->last_send_time_ = millis();
  this->check_dev_inf_sign_ = true;
  this->sg_start_query_data_ = STANDARD_FUNCTION_QUERY_PRODUCT_MODE;
  this->sg_data_len_ = 0;
  this->sg_frame_len_ = 0;
  this->sg_recv_data_state_ = FRAME_IDLE;
  this->s_output_info_switch_flag_ = OUTPUT_SWITCH_INIT;

  memset(this->c_product_mode_, 0, PRODUCT_BUF_MAX_SIZE);
  memset(this->c_product_id_, 0, PRODUCT_BUF_MAX_SIZE);
  memset(this->c_firmware_version_, 0, PRODUCT_BUF_MAX_SIZE);
  memset(this->c_hardware_model_, 0, PRODUCT_BUF_MAX_SIZE);
  memset(this->sg_frame_prase_buf_, 0, FRAME_BUF_MAX_SIZE);
  memset(this->sg_frame_buf_, 0, FRAME_BUF_MAX_SIZE);

  this->set_interval(8000, [this]() {
    if (this->sg_start_query_data_ > UNDERLY_FUNCTION_QUERY_TARGET_MOVEMENT_SPEED ||
        millis() - this->last_recv_time_ > 8000) {
      this->periodic_poll_();
    }
  });
  ESP_LOGCONFIG(TAG, "Set up MR24HPC1 complete");
}

// Timed polling of radar data
void MR24HPC1Component::periodic_poll_() {
  this->get_radar_output_information_switch();  // Query the key status every so often
  this->sg_start_query_data_ = STANDARD_FUNCTION_QUERY_KEEPAWAY_STATUS;
}

// main loop
void MR24HPC1Component::loop() {
  uint8_t byte;

  // Is there data on the serial port
  while (this->available()) {
    this->read_byte(&byte);
    this->r24_split_data_frame_(byte);  // split data frame
    this->last_recv_time_ = millis();
  }

  // Polling Functions, wait till we see nothing on the channel for a bit, then ask for something new
  if (millis() - this->last_recv_time_ > 150 && millis() > this->last_send_time_ + 150 &&
      this->sg_start_query_data_ <= UNDERLY_FUNCTION_QUERY_TARGET_MOVEMENT_SPEED) {
    this->last_send_time_ = millis();
    ESP_LOGD(TAG, "Polling State:%d", this->sg_start_query_data_);
    // Wait till we get something back before asking for something new.
    switch (this->sg_start_query_data_) {
      case STANDARD_FUNCTION_QUERY_PRODUCT_MODE:
        this->get_product_mode();
        break;
      case STANDARD_FUNCTION_QUERY_PRODUCT_ID:
        this->get_product_id();
        break;
      case STANDARD_FUNCTION_QUERY_FIRMWARE_VERSION:
        this->get_firmware_version();
        break;
      case STANDARD_FUNCTION_QUERY_HARDWARE_MODE:
        // Above is the equipment information
        this->get_hardware_model();
        this->check_dev_inf_sign_ = false;
        break;
      case STANDARD_FUNCTION_ENABLE_OPEN_MODE:
        this->set_underlying_open_function(true);
        break;
      case STANDARD_FUNCTION_QUERY_SCENE_MODE:
        this->get_scene_mode();
        break;
      case STANDARD_FUNCTION_QUERY_SENSITIVITY:
        this->get_sensitivity();
        break;
      case STANDARD_FUNCTION_QUERY_UNMANNED_TIME:
        this->get_unmanned_time();
        break;
      case STANDARD_FUNCTION_QUERY_HUMAN_STATUS:
        this->get_human_status();
        break;
      case STANDARD_FUNCTION_QUERY_HUMAN_MOTION_INF:
        this->get_human_motion_info();
        break;
      case STANDARD_FUNCTION_QUERY_BODY_MOVE_PARAMETER:
        this->get_body_motion_params();
        break;
      case STANDARD_FUNCTION_QUERY_KEEPAWAY_STATUS:
        // The above is the basic functional information
        this->get_keep_away();
        break;
      case STANDARD_FUNCTION_QUERY_HEARTBEAT_STATE:
        this->get_heartbeat_packet();
        break;
      case UNDERLY_FUNCTION_QUERY_HUMAN_STATUS:
        this->get_human_status();
        break;
      case UNDERLY_FUNCTION_QUERY_SPATIAL_STATIC_VALUE:
        this->get_spatial_static_value();
        break;
      case UNDERLY_FUNCTION_QUERY_SPATIAL_MOTION_VALUE:
        this->get_spatial_motion_value();
        break;
      case UNDERLY_FUNCTION_QUERY_DISTANCE_OF_STATIC_OBJECT:
        this->get_distance_of_static_object();
        break;
      case UNDERLY_FUNCTION_QUERY_DISTANCE_OF_MOVING_OBJECT:
        this->get_distance_of_moving_object();
        break;
      case UNDERLY_FUNCTION_QUERY_TARGET_MOVEMENT_SPEED:
        this->get_target_movement_speed();
        break;
      default:
        break;
    }
    this->sg_start_query_data_++;
  }
}

// Calculate CRC check digit
static uint8_t get_frame_crc_sum(const uint8_t *data, int len) {
  unsigned int crc_sum = 0;
  for (int i = 0; i < len - 3; i++) {
    crc_sum += data[i];
  }
  return crc_sum & 0xff;
}

// Check that the check digit is correct
static int get_frame_check_status(uint8_t *data, int len) {
  uint8_t crc_sum = get_frame_crc_sum(data, len);
  uint8_t verified = data[len - 3];
  return (verified == crc_sum) ? 1 : 0;
}

// split data frame
void MR24HPC1Component::r24_split_data_frame_(uint8_t value) {
  switch (this->sg_recv_data_state_) {
    case FRAME_IDLE:  // starting value
      if (FRAME_HEADER1_VALUE == value) {
        this->sg_recv_data_state_ = FRAME_HEADER2;
      }
      break;
    case FRAME_HEADER2:
      if (FRAME_HEADER2_VALUE == value) {
        this->sg_frame_buf_[0] = FRAME_HEADER1_VALUE;
        this->sg_frame_buf_[1] = FRAME_HEADER2_VALUE;
        this->sg_recv_data_state_ = FRAME_CTL_WORD;
      } else {
        this->sg_recv_data_state_ = FRAME_IDLE;
        ESP_LOGD(TAG, "FRAME_IDLE ERROR value:%x", value);
      }
      break;
    case FRAME_CTL_WORD:
      this->sg_frame_buf_[2] = value;
      this->sg_recv_data_state_ = FRAME_CMD_WORD;
      break;
    case FRAME_CMD_WORD:
      this->sg_frame_buf_[3] = value;
      this->sg_recv_data_state_ = FRAME_DATA_LEN_H;
      break;
    case FRAME_DATA_LEN_H:
      if (value <= 4) {
        this->sg_data_len_ = value * 256;
        this->sg_frame_buf_[4] = value;
        this->sg_recv_data_state_ = FRAME_DATA_LEN_L;
      } else {
        this->sg_data_len_ = 0;
        this->sg_recv_data_state_ = FRAME_IDLE;
        ESP_LOGD(TAG, "FRAME_DATA_LEN_H ERROR value:%x", value);
      }
      break;
    case FRAME_DATA_LEN_L:
      this->sg_data_len_ += value;
      if (this->sg_data_len_ > 32) {
        ESP_LOGD(TAG, "len=%d, FRAME_DATA_LEN_L ERROR value:%x", this->sg_data_len_, value);
        this->sg_data_len_ = 0;
        this->sg_recv_data_state_ = FRAME_IDLE;
      } else {
        this->sg_frame_buf_[5] = value;
        this->sg_frame_len_ = 6;
        this->sg_recv_data_state_ = FRAME_DATA_BYTES;
      }
      break;
    case FRAME_DATA_BYTES:
      this->sg_data_len_ -= 1;
      this->sg_frame_buf_[this->sg_frame_len_++] = value;
      if (this->sg_data_len_ <= 0) {
        this->sg_recv_data_state_ = FRAME_DATA_CRC;
      }
      break;
    case FRAME_DATA_CRC:
      this->sg_frame_buf_[this->sg_frame_len_++] = value;
      this->sg_recv_data_state_ = FRAME_TAIL1;
      break;
    case FRAME_TAIL1:
      if (FRAME_TAIL1_VALUE == value) {
        this->sg_recv_data_state_ = FRAME_TAIL2;
      } else {
        this->sg_recv_data_state_ = FRAME_IDLE;
        this->sg_frame_len_ = 0;
        this->sg_data_len_ = 0;
        ESP_LOGD(TAG, "FRAME_TAIL1 ERROR value:%x", value);
      }
      break;
    case FRAME_TAIL2:
      if (FRAME_TAIL2_VALUE == value) {
        this->sg_frame_buf_[this->sg_frame_len_++] = FRAME_TAIL1_VALUE;
        this->sg_frame_buf_[this->sg_frame_len_++] = FRAME_TAIL2_VALUE;
        memcpy(this->sg_frame_prase_buf_, this->sg_frame_buf_, this->sg_frame_len_);
        if (get_frame_check_status(this->sg_frame_prase_buf_, this->sg_frame_len_)) {
          this->r24_parse_data_frame_(this->sg_frame_prase_buf_, this->sg_frame_len_);
        } else {
          ESP_LOGD(TAG, "frame check failer!");
        }
      } else {
        ESP_LOGD(TAG, "FRAME_TAIL2 ERROR value:%x", value);
      }
      memset(this->sg_frame_prase_buf_, 0, FRAME_BUF_MAX_SIZE);
      memset(this->sg_frame_buf_, 0, FRAME_BUF_MAX_SIZE);
      this->sg_frame_len_ = 0;
      this->sg_data_len_ = 0;
      this->sg_recv_data_state_ = FRAME_IDLE;
      break;
    default:
      this->sg_recv_data_state_ = FRAME_IDLE;
  }
}

// Parses data frames related to product information
void MR24HPC1Component::r24_frame_parse_product_information_(uint8_t *data) {
  uint16_t product_len = encode_uint16(data[FRAME_COMMAND_WORD_INDEX + 1], data[FRAME_COMMAND_WORD_INDEX + 2]);
  if (data[FRAME_COMMAND_WORD_INDEX] == COMMAND_PRODUCT_MODE) {
    if ((this->product_model_text_sensor_ != nullptr) && (product_len < PRODUCT_BUF_MAX_SIZE)) {
      memset(this->c_product_mode_, 0, PRODUCT_BUF_MAX_SIZE);
      memcpy(this->c_product_mode_, &data[FRAME_DATA_INDEX], product_len);
      this->product_model_text_sensor_->publish_state(this->c_product_mode_);
    } else {
      ESP_LOGD(TAG, "Reply: get product_mode error!");
    }
  } else if (data[FRAME_COMMAND_WORD_INDEX] == COMMAND_PRODUCT_ID) {
    if ((this->product_id_text_sensor_ != nullptr) && (product_len < PRODUCT_BUF_MAX_SIZE)) {
      memset(this->c_product_id_, 0, PRODUCT_BUF_MAX_SIZE);
      memcpy(this->c_product_id_, &data[FRAME_DATA_INDEX], product_len);
      this->product_id_text_sensor_->publish_state(this->c_product_id_);
    } else {
      ESP_LOGD(TAG, "Reply: get productId error!");
    }
  } else if (data[FRAME_COMMAND_WORD_INDEX] == COMMAND_HARDWARE_MODEL) {
    if ((this->hardware_model_text_sensor_ != nullptr) && (product_len < PRODUCT_BUF_MAX_SIZE)) {
      memset(this->c_hardware_model_, 0, PRODUCT_BUF_MAX_SIZE);
      memcpy(this->c_hardware_model_, &data[FRAME_DATA_INDEX], product_len);
      this->hardware_model_text_sensor_->publish_state(this->c_hardware_model_);
      ESP_LOGD(TAG, "Reply: get hardware_model :%s", this->c_hardware_model_);
    } else {
      ESP_LOGD(TAG, "Reply: get hardwareModel error!");
    }
  } else if (data[FRAME_COMMAND_WORD_INDEX] == COMMAND_FIRMWARE_VERSION) {
    if ((this->firware_version_text_sensor_ != nullptr) && (product_len < PRODUCT_BUF_MAX_SIZE)) {
      memset(this->c_firmware_version_, 0, PRODUCT_BUF_MAX_SIZE);
      memcpy(this->c_firmware_version_, &data[FRAME_DATA_INDEX], product_len);
      this->firware_version_text_sensor_->publish_state(this->c_firmware_version_);
    } else {
      ESP_LOGD(TAG, "Reply: get firmwareVersion error!");
    }
  }
}

// Parsing the underlying open parameters
void MR24HPC1Component::r24_frame_parse_open_underlying_information_(uint8_t *data) {
  if (data[FRAME_COMMAND_WORD_INDEX] == 0x00) {
    if (data[FRAME_DATA_INDEX]) {
      this->s_output_info_switch_flag_ = OUTPUT_SWITCH_ON;
    } else {
      this->s_output_info_switch_flag_ = OUTPUT_SWTICH_OFF;
    }
  } else if (data[FRAME_COMMAND_WORD_INDEX] == 0x01) {
    ESP_LOGD(TAG, "Custom stuff: %d ", data[FRAME_DATA_INDEX]);
    if (this->existence_energy_sensor_ != nullptr) {
      this->existence_energy_sensor_->publish_state(data[FRAME_DATA_INDEX]);
    }
    if (this->static_distance_sensor_ != nullptr) {
      this->static_distance_sensor_->publish_state(S_BOUNDARY_MTR[data[FRAME_DATA_INDEX + 1]]);
    }
    if (this->motion_energy_sensor_ != nullptr) {
      this->motion_energy_sensor_->publish_state(data[FRAME_DATA_INDEX + 2]);
    }
    if (this->motion_distance_sensor_ != nullptr) {
      this->motion_distance_sensor_->publish_state(S_BOUNDARY_MTR[data[FRAME_DATA_INDEX + 3]]);
    }
    if (this->motion_speed_sensor_ != nullptr) {
      this->motion_speed_sensor_->publish_state(data[FRAME_DATA_INDEX + 4]);
    }
  } else if ((data[FRAME_COMMAND_WORD_INDEX] == 0x06) || (data[FRAME_COMMAND_WORD_INDEX] == 0x86)) {
    // none:0x00  close_to:0x01  far_away:0x02
    if ((this->keep_away_text_sensor_ != nullptr) && (data[FRAME_DATA_INDEX] < 3)) {
      this->keep_away_text_sensor_->publish_state(S_KEEP_AWAY_STR[data[FRAME_DATA_INDEX]]);
    }
  } else if ((this->movement_signs_sensor_ != nullptr) &&
             ((data[FRAME_COMMAND_WORD_INDEX] == 0x07) || (data[FRAME_COMMAND_WORD_INDEX] == 0x87))) {
    ESP_LOGD(TAG, "Movement signs: %d ", data[FRAME_DATA_INDEX]);
    this->movement_signs_sensor_->publish_state(data[FRAME_DATA_INDEX]);
  } else if (((data[FRAME_COMMAND_WORD_INDEX] == 0x08) || (data[FRAME_COMMAND_WORD_INDEX] == 0x88))) {
    ESP_LOGD(TAG, "Existance threshold: %d %p", data[FRAME_DATA_INDEX], this->existence_threshold_number_);
    if (this->existence_threshold_number_ != nullptr) {
      this->existence_threshold_number_->publish_state(data[FRAME_DATA_INDEX]);
    }
  } else if (((data[FRAME_COMMAND_WORD_INDEX] == 0x09) || (data[FRAME_COMMAND_WORD_INDEX] == 0x89))) {
    ESP_LOGD(TAG, "Motion threshold: %d %p", data[FRAME_DATA_INDEX], this->motion_threshold_number_);
    if (this->motion_threshold_number_ != nullptr) {
      this->motion_threshold_number_->publish_state(data[FRAME_DATA_INDEX]);
    }
  } else if (((data[FRAME_COMMAND_WORD_INDEX] == 0x0b) || (data[FRAME_COMMAND_WORD_INDEX] == 0x8b))) {
    ESP_LOGD(TAG, "Motion boundary index: %d %p", data[FRAME_DATA_INDEX], this->motion_boundary_select_);
    if (this->motion_boundary_select_ != nullptr) {
      this->motion_boundary_select_->publish_state(S_BOUNDARY_STR[data[FRAME_DATA_INDEX] - 1]);
    }
  } else if (((data[FRAME_COMMAND_WORD_INDEX] == 0x0c) || (data[FRAME_COMMAND_WORD_INDEX] == 0x8c))) {
    ESP_LOGD(TAG, "Motion trigger time: %d %d %d %d %p", data[FRAME_DATA_INDEX], data[FRAME_DATA_INDEX + 1],
             data[FRAME_DATA_INDEX + 2], data[FRAME_DATA_INDEX + 3]);
    uint32_t motion_trigger_time = encode_uint32(data[FRAME_DATA_INDEX], data[FRAME_DATA_INDEX + 1],
                                                 data[FRAME_DATA_INDEX + 2], data[FRAME_DATA_INDEX + 3]);
    if (this->motion_trigger_number_ != nullptr) {
      this->motion_trigger_number_->publish_state(motion_trigger_time);
    }
  } else if (((data[FRAME_COMMAND_WORD_INDEX] == 0x0d) || (data[FRAME_COMMAND_WORD_INDEX] == 0x8d))) {
    ESP_LOGD(TAG, "Move to rest time: %d %d %d %d %p", data[FRAME_DATA_INDEX], data[FRAME_DATA_INDEX + 1],
             data[FRAME_DATA_INDEX + 2], data[FRAME_DATA_INDEX + 3], this->motion_to_rest_number_);
    uint32_t move_to_rest_time = encode_uint32(data[FRAME_DATA_INDEX], data[FRAME_DATA_INDEX + 1],
                                               data[FRAME_DATA_INDEX + 2], data[FRAME_DATA_INDEX + 3]);
    if (this->motion_to_rest_number_ != nullptr) {
      this->motion_to_rest_number_->publish_state(move_to_rest_time);
    }
  } else if (data[FRAME_COMMAND_WORD_INDEX] == 0x80) {
    if (data[FRAME_DATA_INDEX]) {
      this->s_output_info_switch_flag_ = OUTPUT_SWITCH_ON;
    } else {
      this->s_output_info_switch_flag_ = OUTPUT_SWTICH_OFF;
    }
  }
}

void MR24HPC1Component::r24_parse_data_frame_(uint8_t *data, uint8_t len) {
  switch (data[FRAME_CONTROL_WORD_INDEX]) {
    case 0x01: {
      if ((this->heartbeat_state_text_sensor_ != nullptr) && (data[FRAME_COMMAND_WORD_INDEX] == 0x01)) {
        this->heartbeat_state_text_sensor_->publish_state("Equipment Normal");
      } else if (data[FRAME_COMMAND_WORD_INDEX] == 0x02) {
        ESP_LOGD(TAG, "Reply: query restart packet");
      } else if (this->heartbeat_state_text_sensor_ != nullptr) {
        this->heartbeat_state_text_sensor_->publish_state("Equipment Abnormal");
      }
    } break;
    case 0x02: {
      this->r24_frame_parse_product_information_(data);
    } break;
    case 0x05: {
      this->r24_frame_parse_work_status_(data);
    } break;
    case 0x08: {
      this->r24_frame_parse_open_underlying_information_(data);
    } break;
    case 0x80: {
      this->r24_frame_parse_human_information_(data);
    } break;
    default:
      ESP_LOGD(TAG, "control word:0x%02X not found", data[FRAME_CONTROL_WORD_INDEX]);
      break;
  }
}

void MR24HPC1Component::r24_frame_parse_work_status_(uint8_t *data) {
  if (data[FRAME_COMMAND_WORD_INDEX] == 0x01) {
    ESP_LOGD(TAG, "Reply: get radar init status 0x%02X", data[FRAME_DATA_INDEX]);
  } else if (data[FRAME_COMMAND_WORD_INDEX] == 0x07) {
    if ((this->scene_mode_select_ != nullptr) && (this->scene_mode_select_->has_index(data[FRAME_DATA_INDEX]))) {
      this->scene_mode_select_->publish_state(S_SCENE_STR[data[FRAME_DATA_INDEX]]);
    } else {
      ESP_LOGD(TAG, "Select has index offset %d", data[FRAME_DATA_INDEX]);
    }
  } else if (((data[FRAME_COMMAND_WORD_INDEX] == 0x08) || (data[FRAME_COMMAND_WORD_INDEX] == 0x88))) {
    // 1-3
    ESP_LOGD(TAG, "Reply: get sensitivity 0x%02X", data[FRAME_DATA_INDEX]);
    if (this->sensitivity_number_ != nullptr) {
      this->sensitivity_number_->publish_state(data[FRAME_DATA_INDEX]);
    }
  } else if (data[FRAME_COMMAND_WORD_INDEX] == 0x09) {
    // 1-4
  } else if (data[FRAME_COMMAND_WORD_INDEX] == 0x81) {
    ESP_LOGD(TAG, "Reply: get radar init status 0x%02X", data[FRAME_DATA_INDEX]);
  } else if (data[FRAME_COMMAND_WORD_INDEX] == 0x87) {
    ESP_LOGD(TAG, "Select has index offset %d ", data[FRAME_DATA_INDEX]);
    if ((this->scene_mode_select_ != nullptr) && (this->scene_mode_select_->has_index(data[FRAME_DATA_INDEX]))) {
      this->scene_mode_select_->publish_state(S_SCENE_STR[data[FRAME_DATA_INDEX]]);
    }
  } else if ((data[FRAME_COMMAND_WORD_INDEX] == 0x88)) {
    ESP_LOGD(TAG, "Reply: get sensitivity 0x%02X", data[FRAME_DATA_INDEX]);
    if (this->sensitivity_number_ != nullptr) {
      this->sensitivity_number_->publish_state(data[FRAME_DATA_INDEX]);
    }
  } else if ((data[FRAME_COMMAND_WORD_INDEX] == 0x89)) {
  } else {
    ESP_LOGD(TAG, "[%s] No found COMMAND_WORD(%02X) in Frame", __FUNCTION__, data[FRAME_COMMAND_WORD_INDEX]);
  }
}

void MR24HPC1Component::r24_frame_parse_human_information_(uint8_t *data) {
  if ((this->has_target_binary_sensor_ != nullptr) &&
      ((data[FRAME_COMMAND_WORD_INDEX] == 0x01) || (data[FRAME_COMMAND_WORD_INDEX] == 0x81))) {
    this->has_target_binary_sensor_->publish_state(S_SOMEONE_EXISTS_STR[data[FRAME_DATA_INDEX]]);
    ESP_LOGD(TAG, "Binary sensor state %d ", data[FRAME_DATA_INDEX]);
  } else if ((this->motion_status_text_sensor_ != nullptr) &&
             ((data[FRAME_COMMAND_WORD_INDEX] == 0x02) || (data[FRAME_COMMAND_WORD_INDEX] == 0x82))) {
    if (data[FRAME_DATA_INDEX] < 3) {
      this->motion_status_text_sensor_->publish_state(S_MOTION_STATUS_STR[data[FRAME_DATA_INDEX]]);
    }
    ESP_LOGD(TAG, "Motion sensor state %d %p", data[FRAME_DATA_INDEX], this->motion_status_text_sensor_);
  } else if (((data[FRAME_COMMAND_WORD_INDEX] == 0x03) || (data[FRAME_COMMAND_WORD_INDEX] == 0x83))) {
    ESP_LOGD(TAG, "Movement signs sensor state %d %p ", data[FRAME_DATA_INDEX], this->movement_signs_sensor_);
    if (this->movement_signs_sensor_ != nullptr) {
      this->movement_signs_sensor_->publish_state(data[FRAME_DATA_INDEX]);
    }
  } else if (((data[FRAME_COMMAND_WORD_INDEX] == 0x0A) || (data[FRAME_COMMAND_WORD_INDEX] == 0x8A))) {
    // none:0x00  1s:0x01 30s:0x02 1min:0x03 2min:0x04 5min:0x05 10min:0x06 30min:0x07 1hour:0x08
    ESP_LOGD(TAG, "Motion timeout select %d %p", data[FRAME_DATA_INDEX], this->motion_timeout_select_);
    if (data[FRAME_DATA_INDEX] < 9) {
      if (this->motion_timeout_select_ != nullptr) {
        this->motion_timeout_select_->publish_state(S_UNMANNED_TIME_STR[data[FRAME_DATA_INDEX]]);
      }
    }
  } else if (((data[FRAME_COMMAND_WORD_INDEX] == 0x0B) || (data[FRAME_COMMAND_WORD_INDEX] == 0x8B))) {
    // none:0x00  close_to:0x01  far_away:0x02
    if (data[FRAME_DATA_INDEX] < 3 && this->keep_away_text_sensor_ != nullptr) {
      this->keep_away_text_sensor_->publish_state(S_KEEP_AWAY_STR[data[FRAME_DATA_INDEX]]);
    }
    ESP_LOGD(TAG, "Keep Away sensor %d %p", data[FRAME_DATA_INDEX], this->keep_away_text_sensor_);
  } else {
    ESP_LOGD(TAG, "[%s] No found COMMAND_WORD(%02X) in Frame", __FUNCTION__, data[FRAME_COMMAND_WORD_INDEX]);
  }
}

// Sending data frames
void MR24HPC1Component::send_query_(const uint8_t *query, size_t string_length) {
  this->write_array(query, string_length);
}

// Send Heartbeat Packet Command
void MR24HPC1Component::get_heartbeat_packet() {
  ESP_LOGD(TAG, "get_heartbeat_packet");
  this->send_query_(GET_HEARTBEAT, sizeof(GET_HEARTBEAT));
}

// Issuance of the underlying open parameter query command
void MR24HPC1Component::get_radar_output_information_switch() {
  ESP_LOGD(TAG, "get_radar_output_information_switch");
  this->send_query_(GET_RADAR_OUTPUT_INFORMATION_SWITCH, sizeof(GET_RADAR_OUTPUT_INFORMATION_SWITCH));
}

// Issuance of product model orders
void MR24HPC1Component::get_product_mode() {
  ESP_LOGD(TAG, "get_product_mode");
  this->send_query_(GET_PRODUCT_MODE, sizeof(GET_PRODUCT_MODE));
}

// Issuing the Get Product ID command
void MR24HPC1Component::get_product_id() {
  ESP_LOGD(TAG, "get_product_id");
  this->send_query_(GET_PRODUCT_ID, sizeof(GET_PRODUCT_ID));
}

// Issuing hardware model commands
void MR24HPC1Component::get_hardware_model() {
  ESP_LOGD(TAG, "get_hardware_model");
  this->send_query_(GET_HARDWARE_MODEL, sizeof(GET_HARDWARE_MODEL));
}

// Issuing software version commands
void MR24HPC1Component::get_firmware_version() {
  ESP_LOGD(TAG, "get_firmware_version");
  this->send_query_(GET_FIRMWARE_VERSION, sizeof(GET_FIRMWARE_VERSION));
}

void MR24HPC1Component::get_human_status() {
  ESP_LOGD(TAG, "get_human_status");
  this->send_query_(GET_HUMAN_STATUS, sizeof(GET_HUMAN_STATUS));
}

void MR24HPC1Component::get_human_motion_info() {
  ESP_LOGD(TAG, "get_human_motion_info");

  this->send_query_(GET_HUMAN_MOTION_INFORMATION, sizeof(GET_HUMAN_MOTION_INFORMATION));
}

void MR24HPC1Component::get_body_motion_params() {
  ESP_LOGD(TAG, "get_body_motion_params");
  this->send_query_(GET_BODY_MOTION_PARAMETERS, sizeof(GET_BODY_MOTION_PARAMETERS));
}

void MR24HPC1Component::get_keep_away() {
  ESP_LOGD(TAG, "get_keep_away");
  this->send_query_(GET_KEEP_AWAY, sizeof(GET_KEEP_AWAY));
}

void MR24HPC1Component::get_scene_mode() {
  ESP_LOGD(TAG, "get_scene_mode");
  this->send_query_(GET_SCENE_MODE, sizeof(GET_SCENE_MODE));
}

void MR24HPC1Component::get_sensitivity() {
  ESP_LOGD(TAG, "get_sensitivity");
  this->send_query_(GET_SENSITIVITY, sizeof(GET_SENSITIVITY));
}

void MR24HPC1Component::get_unmanned_time() {
  ESP_LOGD(TAG, "get_unmanned_time");
  this->send_query_(GET_UNMANNED_TIME, sizeof(GET_UNMANNED_TIME));
}

void MR24HPC1Component::get_existence_boundary() {
  ESP_LOGD(TAG, "get_existence_boundary");
  this->send_query_(GET_EXISTENCE_BOUNDARY, sizeof(GET_EXISTENCE_BOUNDARY));
}

void MR24HPC1Component::get_motion_boundary() {
  ESP_LOGD(TAG, "get_motion_boundary");
  this->send_query_(GET_MOTION_BOUNDARY, sizeof(GET_MOTION_BOUNDARY));
}

void MR24HPC1Component::get_spatial_static_value() {
  ESP_LOGD(TAG, "get_spatial_static_value");
  this->send_query_(GET_SPATIAL_STATIC_VALUE, sizeof(GET_SPATIAL_STATIC_VALUE));
}

void MR24HPC1Component::get_spatial_motion_value() {
  ESP_LOGD(TAG, "get_spatial_motion_value");
  this->send_query_(GET_SPATIAL_MOTION_VALUE, sizeof(GET_SPATIAL_MOTION_VALUE));
}

void MR24HPC1Component::get_distance_of_static_object() {
  ESP_LOGD(TAG, "get_distance_of_static_object");
  this->send_query_(GET_DISTANCE_OF_STATIC_OBJECT, sizeof(GET_DISTANCE_OF_STATIC_OBJECT));
}

void MR24HPC1Component::get_distance_of_moving_object() {
  ESP_LOGD(TAG, "get_distance_of_moving_object");
  this->send_query_(GET_DISTANCE_OF_MOVING_OBJECT, sizeof(GET_DISTANCE_OF_MOVING_OBJECT));
}

void MR24HPC1Component::get_target_movement_speed() {
  ESP_LOGD(TAG, "get_target_movement_speed");
  this->send_query_(GET_TARGET_MOVEMENT_SPEED, sizeof(GET_TARGET_MOVEMENT_SPEED));
}

void MR24HPC1Component::get_existence_threshold() {
  ESP_LOGD(TAG, "get_existence_threshold");
  this->send_query_(GET_EXISTENCE_THRESHOLD, sizeof(GET_EXISTENCE_THRESHOLD));
}

void MR24HPC1Component::get_motion_threshold() {
  ESP_LOGD(TAG, "get_motion_threshold");
  this->send_query_(GET_MOTION_THRESHOLD, sizeof(GET_MOTION_THRESHOLD));
}

void MR24HPC1Component::get_motion_trigger_time() {
  ESP_LOGD(TAG, "get_motion_trigger_time");
  this->send_query_(GET_MOTION_TRIGGER_TIME, sizeof(GET_MOTION_TRIGGER_TIME));
}

void MR24HPC1Component::get_motion_to_rest_time() {
  ESP_LOGD(TAG, "get_motion_to_rest_time");
  this->send_query_(GET_MOTION_TO_REST_TIME, sizeof(GET_MOTION_TO_REST_TIME));
}

void MR24HPC1Component::get_custom_unman_time() {
  ESP_LOGD(TAG, "get_custom_unman_time");
  this->send_query_(GET_CUSTOM_UNMAN_TIME, sizeof(GET_CUSTOM_UNMAN_TIME));
}

// Logic of setting: After setting, query whether the setting is successful or not!

void MR24HPC1Component::set_underlying_open_function(bool enable) {
  ESP_LOGD(TAG, "set_underlying_open_function");
  if (enable) {
    this->send_query_(UNDERLYING_SWITCH_ON, sizeof(UNDERLYING_SWITCH_ON));
  } else {
    this->send_query_(UNDERLYING_SWITCH_OFF, sizeof(UNDERLYING_SWITCH_OFF));
  }
  if (this->keep_away_text_sensor_ != nullptr) {
    this->keep_away_text_sensor_->publish_state("");
  }
  if (this->motion_status_text_sensor_ != nullptr) {
    this->motion_status_text_sensor_->publish_state("");
  }
}

void MR24HPC1Component::set_scene_mode(uint8_t value) {
  ESP_LOGD(TAG, "set_scene_mode");
  uint8_t send_data_len = 10;
  uint8_t send_data[10] = {0x53, 0x59, 0x05, 0x07, 0x00, 0x01, value, 0x00, 0x54, 0x43};
  send_data[7] = get_frame_crc_sum(send_data, send_data_len);
  this->send_query_(send_data, send_data_len);
  this->get_scene_mode();
  this->get_sensitivity();
  this->get_existence_boundary();
  this->get_motion_boundary();
  this->get_existence_threshold();
  this->get_motion_threshold();
  this->get_motion_trigger_time();
  this->get_motion_to_rest_time();
  this->get_custom_unman_time();
}

void MR24HPC1Component::set_sensitivity(uint8_t value) {
  ESP_LOGD(TAG, "set_sensitivity");
  if (value == 0x00)
    return;
  uint8_t send_data_len = 10;
  uint8_t send_data[10] = {0x53, 0x59, 0x05, 0x08, 0x00, 0x01, value, 0x00, 0x54, 0x43};
  send_data[7] = get_frame_crc_sum(send_data, send_data_len);
  this->send_query_(send_data, send_data_len);
  this->get_scene_mode();
  this->get_sensitivity();
}

void MR24HPC1Component::set_restart() {
  ESP_LOGD(TAG, "set_restart");
  this->send_query_(SET_RESTART, sizeof(SET_RESTART));
  this->check_dev_inf_sign_ = true;
  this->sg_start_query_data_ = STANDARD_FUNCTION_QUERY_PRODUCT_MODE;
}

void MR24HPC1Component::set_unman_time(uint8_t value) {
  ESP_LOGD(TAG, "set_unman_time");
  uint8_t send_data_len = 10;
  uint8_t send_data[10] = {0x53, 0x59, 0x80, 0x0a, 0x00, 0x01, value, 0x00, 0x54, 0x43};
  send_data[7] = get_frame_crc_sum(send_data, send_data_len);
  this->send_query_(send_data, send_data_len);
  this->get_unmanned_time();
}

void MR24HPC1Component::set_existence_boundary(uint8_t value) {
  ESP_LOGD(TAG, "set_existence_boundary");
  uint8_t send_data_len = 10;
  uint8_t send_data[10] = {0x53, 0x59, 0x08, 0x0A, 0x00, 0x01, (uint8_t) (value + 1), 0x00, 0x54, 0x43};
  send_data[7] = get_frame_crc_sum(send_data, send_data_len);
  this->send_query_(send_data, send_data_len);
  this->get_existence_boundary();
}

void MR24HPC1Component::set_motion_boundary(uint8_t value) {
  ESP_LOGD(TAG, "set_motion_boundary");
  uint8_t send_data_len = 10;
  uint8_t send_data[10] = {0x53, 0x59, 0x08, 0x0B, 0x00, 0x01, (uint8_t) (value + 1), 0x00, 0x54, 0x43};
  send_data[7] = get_frame_crc_sum(send_data, send_data_len);
  this->send_query_(send_data, send_data_len);
  this->get_motion_boundary();
}

void MR24HPC1Component::set_existence_threshold(uint8_t value) {
  ESP_LOGD(TAG, "set_existence_threshold");
  uint8_t send_data_len = 10;
  uint8_t send_data[10] = {0x53, 0x59, 0x08, 0x08, 0x00, 0x01, value, 0x00, 0x54, 0x43};
  send_data[7] = get_frame_crc_sum(send_data, send_data_len);
  this->send_query_(send_data, send_data_len);
  this->get_existence_threshold();
}

void MR24HPC1Component::set_motion_threshold(uint8_t value) {
  ESP_LOGD(TAG, "set_motion_threshold");
  uint8_t send_data_len = 10;
  uint8_t send_data[10] = {0x53, 0x59, 0x08, 0x09, 0x00, 0x01, value, 0x00, 0x54, 0x43};
  send_data[7] = get_frame_crc_sum(send_data, send_data_len);
  this->send_query_(send_data, send_data_len);
  this->get_motion_threshold();
}

void MR24HPC1Component::set_motion_trigger_time(uint8_t value) {
  ESP_LOGD(TAG, "set_motion_trigger_time");
  uint8_t send_data_len = 13;
  uint8_t send_data[13] = {0x53, 0x59, 0x08, 0x0C, 0x00, 0x04, 0x00, 0x00, 0x00, value, 0x00, 0x54, 0x43};
  send_data[10] = get_frame_crc_sum(send_data, send_data_len);
  this->send_query_(send_data, send_data_len);
  this->get_motion_trigger_time();
}

void MR24HPC1Component::set_motion_to_rest_time(uint16_t value) {
  ESP_LOGD(TAG, "set_motion_to_rest_time");
  uint8_t h8_num = (value >> 8) & 0xff;
  uint8_t l8_num = value & 0xff;
  uint8_t send_data_len = 13;
  uint8_t send_data[13] = {0x53, 0x59, 0x08, 0x0D, 0x00, 0x04, 0x00, 0x00, h8_num, l8_num, 0x00, 0x54, 0x43};
  send_data[10] = get_frame_crc_sum(send_data, send_data_len);
  this->send_query_(send_data, send_data_len);
  this->get_motion_to_rest_time();
}

void MR24HPC1Component::set_custom_unman_time(uint16_t value) {
  ESP_LOGD(TAG, "set_custom_unman_time");
  uint32_t value_ms = value * 1000;
  uint8_t h24_num = (value_ms >> 24) & 0xff;
  uint8_t h16_num = (value_ms >> 16) & 0xff;
  uint8_t h8_num = (value_ms >> 8) & 0xff;
  uint8_t l8_num = value_ms & 0xff;
  uint8_t send_data_len = 13;
  uint8_t send_data[13] = {0x53, 0x59, 0x08, 0x0E, 0x00, 0x04, h24_num, h16_num, h8_num, l8_num, 0x00, 0x54, 0x43};
  send_data[10] = get_frame_crc_sum(send_data, send_data_len);
  this->send_query_(send_data, send_data_len);
  this->get_custom_unman_time();
}

}  // namespace seeed_mr24hpc1
}  // namespace esphome
