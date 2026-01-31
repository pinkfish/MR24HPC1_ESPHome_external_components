import esphome.codegen as cg
from esphome.components import select
import esphome.config_validation as cv
from esphome.const import (
    ENTITY_CATEGORY_CONFIG,
)
from .. import CONF_MR24HPC1_ID, MR24HPC1Component, mr24hpc1_ns

SceneModeSelect = mr24hpc1_ns.class_("SceneModeSelect", select.Select)
MotionTimeoutSelect = mr24hpc1_ns.class_("MotionTimeoutSelect", select.Select)

CONF_SCENE_MODE = "scene_mode"
CONF_MOTION_TIMEOUT = "motion_timeout"

CONFIG_SCHEMA = {
    cv.GenerateID(CONF_MR24HPC1_ID): cv.use_id(MR24HPC1Component),
    cv.Optional(CONF_SCENE_MODE): select.select_schema(
        SceneModeSelect,
        entity_category=ENTITY_CATEGORY_CONFIG,
        icon="mdi:hoop-house",
    ),
    cv.Optional(CONF_MOTION_TIMEOUT): select.select_schema(
        MotionTimeoutSelect,
        entity_category=ENTITY_CATEGORY_CONFIG,
        icon="mdi:human-greeting-variant",
    ),
}


async def to_code(config):
    mr24hpc1_component = await cg.get_variable(config[CONF_MR24HPC1_ID])
    if scenemode_config := config.get(CONF_SCENE_MODE):
        s = await select.new_select(
            scenemode_config,
            options=["None", "Living Room", "Bedroom", "Washroom", "Area Detection"],
        )
        await cg.register_parented(s, config[CONF_MR24HPC1_ID])
        cg.add(mr24hpc1_component.set_scene_mode_select(s))
    if motiontimeout_config := config.get(CONF_MOTION_TIMEOUT):
        s = await select.new_select(
            motiontimeout_config,
            options=[
                "None",
                "10s",
                "30s",
                "1min",
                "2min",
                "5min",
                "10min",
                "30min",
                "60min",
            ],
        )
        await cg.register_parented(s, config[CONF_MR24HPC1_ID])
        cg.add(mr24hpc1_component.set_motion_timeout_select(s))
