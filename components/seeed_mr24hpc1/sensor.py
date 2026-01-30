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
CONF_EXISTENCE_ENERGY = "existence_energy"
CONF_MOTION_ENERGY = "motion_energy"
CONF_MOTION_SPEED = "motion_speed"
CONF_STATIC_DISTANCE = "static_distance"
CONF_MOTION_DISTANCE = "motion_distance"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_MR24HPC1_ID): cv.use_id(MR24HPC1Component),
        cv.Optional(CONF_MOVEMENT_SIGNS): sensor.sensor_schema(
            icon="mdi:human-greeting-variant",
        ),
        cv.Optional(CONF_EXISTENCE_ENERGY): sensor.sensor_schema(
            device_class=DEVICE_CLASS_ENERGY,
            unit_of_measurement="Wh",
        ),
        cv.Optional(CONF_MOTION_ENERGY): sensor.sensor_schema(
            device_class=DEVICE_CLASS_ENERGY,
        ),
        cv.Optional(CONF_MOTION_SPEED): sensor.sensor_schema(
            icon="mdi:run-fast",
        ),
        cv.Optional(CONF_STATIC_DISTANCE): text_sensor.text_sensor_schema(
            entity_category=DEVICE_CLASS_DISTANCE, 
            unit_of_measurement=UNIT_METER,
            icon="mdi:walk"
        ),
        cv.Optional(CONF_MOTION_DISTANCE): text_sensor.text_sensor_schema(
            device_class=DEVICE_CLASS_DISTANCE, 
            unit_of_measurement=UNIT_METER,
            icon="mdi:walk"
        ),
    }
)


async def to_code(config):
    mr24hpc1_component = await cg.get_variable(config[CONF_MR24HPC1_ID])
    if movementsigns_config := config.get(CONF_MOVEMENT_SIGNS):
        sens = await sensor.new_sensor(movementsigns_config)
        cg.add(mr24hpc1_component.set_movement_signs_sensor(sens))
    if existenceenergy_config := config.get(CONF_EXISTENCE_ENERGY):
        sens = await sensor.new_sensor(existenceenergy_config)
        cg.add(mr24hpc1_component.set_existence_energy_sensor(sens))
    if motionenergy_config := config.get(CONF_MOTION_ENERGY):
        sens = await sensor.new_sensor(motionenergy_config)
        cg.add(mr24hpc1_component.set_motion_energy_sensor(sens))
    if motionspeed_config := config.get(CONF_MOTION_SPEED):
        sens = await sensor.new_sensor(motionspeed_config)
        cg.add(mr24hpc1_component.set_motion_speed_sensor(sens))
    if staticdistance_config := config.get(CONF_STATIC_DISTANCE):
        sens = await sensor.new_sensor(staticdistance_config)
        cg.add(mr24hpc1_component.set_static_distance_sensor(sens))
    if motiondistance_config := config.get(CONF_MOTION_DISTANCE):
        sens = await sensor.new_sensor(motiondistance_config)
        cg.add(mr24hpc1_component.set_motion_distance_sensor(sens))
