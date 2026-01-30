import esphome.codegen as cg
from esphome.components import sensor
import esphome.config_validation as cv
from esphome.const import (
    DEVICE_CLASS_DISTANCE,
    DEVICE_CLASS_ENERGY,
    DEVICE_CLASS_SPEED,
    UNIT_METER,
)
from . import CONF_MR24HPC1_ID, MR24HPC1Component

CONF_MOVEMENT_SIGNS = "movement_signs"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_MR24HPC1_ID): cv.use_id(MR24HPC1Component),
        cv.Optional(CONF_MOVEMENT_SIGNS): sensor.sensor_schema(
            icon="mdi:human-greeting-variant",
        ),
    }
)


async def to_code(config):
    mr24hpc1_component = await cg.get_variable(config[CONF_MR24HPC1_ID])
    if movementsigns_config := config.get(CONF_MOVEMENT_SIGNS):
        sens = await sensor.new_sensor(movementsigns_config)
        cg.add(mr24hpc1_component.set_movement_signs_sensor(sens))
