import esphome.codegen as cg
from esphome.components import number
import esphome.config_validation as cv
from esphome.const import (
    CONF_SENSITIVITY,
    ENTITY_CATEGORY_CONFIG,
)
from .. import CONF_MR24HPC1_ID, MR24HPC1Component, mr24hpc1_ns

SensitivityNumber = mr24hpc1_ns.class_("SensitivityNumber", number.Number)
ExistenceThresholdNumber = mr24hpc1_ns.class_("ExistenceThresholdNumber", number.Number)
MotionThresholdNumber = mr24hpc1_ns.class_("MotionThresholdNumber", number.Number)
MotionTriggerTimeNumber = mr24hpc1_ns.class_("MotionTriggerTimeNumber", number.Number)
MotionToRestTimeNumber = mr24hpc1_ns.class_("MotionToRestTimeNumber", number.Number)


CONF_EXISTENCE_THRESHOLD = "existence_threshold"
CONF_MOTION_THRESHOLD = "motion_threshold"
CONF_MOTION_TRIGGER = "motion_trigger"
CONF_MOTION_TO_REST = "motion_to_rest"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_MR24HPC1_ID): cv.use_id(MR24HPC1Component),
        cv.Optional(CONF_SENSITIVITY): number.number_schema(
            SensitivityNumber,
            entity_category=ENTITY_CATEGORY_CONFIG,
            icon="mdi:archive-check-outline",
        ),
        cv.Optional(CONF_EXISTENCE_THRESHOLD): number.number_schema(
            ExistenceThresholdNumber,
            entity_category=ENTITY_CATEGORY_CONFIG,
        ),
        cv.Optional(CONF_MOTION_THRESHOLD): number.number_schema(
            MotionThresholdNumber,
            entity_category=ENTITY_CATEGORY_CONFIG,
        ),
        cv.Optional(CONF_MOTION_TRIGGER): number.number_schema(
            MotionTriggerTimeNumber,
            entity_category=ENTITY_CATEGORY_CONFIG,
            icon="mdi:camera-timer",
            unit_of_measurement="ms",
        ),
        cv.Optional(CONF_MOTION_TO_REST): number.number_schema(
            MotionToRestTimeNumber,
            entity_category=ENTITY_CATEGORY_CONFIG,
            icon="mdi:camera-timer",
            unit_of_measurement="ms",
        ),
    }
)


async def to_code(config):
    mr24hpc1_component = await cg.get_variable(config[CONF_MR24HPC1_ID])
    if sensitivity_config := config.get(CONF_SENSITIVITY):
        n = await number.new_number(
            sensitivity_config,
            min_value=0,
            max_value=3,
            step=1,
        )
        await cg.register_parented(n, mr24hpc1_component)
        cg.add(mr24hpc1_component.set_sensitivity_number(n))
    if existence_threshold_config := config.get(CONF_EXISTENCE_THRESHOLD):
        n = await number.new_number(
            existence_threshold_config,
            min_value=0,
            max_value=250,
            step=1,
        )
        await cg.register_parented(n, mr24hpc1_component)
        cg.add(mr24hpc1_component.set_existence_threshold_number(n))
    if motion_threshold_config := config.get(CONF_MOTION_THRESHOLD):
        n = await number.new_number(
            motion_threshold_config,
            min_value=0,
            max_value=250,
            step=1,
        )
        await cg.register_parented(n, mr24hpc1_component)
        cg.add(mr24hpc1_component.set_motion_threshold_number(n))
    if motion_trigger_config := config.get(CONF_MOTION_TRIGGER):
        n = await number.new_number(
            motion_trigger_config,
            min_value=0,
            max_value=150,
            step=1,
        )
        await cg.register_parented(n, mr24hpc1_component)
        cg.add(mr24hpc1_component.set_motion_trigger_number(n))
    if motion_to_rest_config := config.get(CONF_MOTION_TO_REST):
        n = await number.new_number(
            motion_to_rest_config,
            min_value=0,
            max_value=9,
            step=1,
        )
        await cg.register_parented(n, mr24hpc1_component)
        cg.add(mr24hpc1_component.set_motion_to_rest_number(n))
