import esphome.codegen as cg
from esphome.components import button
import esphome.config_validation as cv
from esphome.const import (
    CONF_RESTART,
    DEVICE_CLASS_RESTART,
    ENTITY_CATEGORY_CONFIG,
    ICON_RESTART_ALERT,
)
from .. import CONF_MR24HPC1PF_ID, MR24HPC1PFComponent, mr24hpc1pf_ns

RestartButton = mr24hpc1pf_ns.class_("RestartButton", button.Button)


CONFIG_SCHEMA = {
    cv.GenerateID(CONF_MR24HPC1PF_ID): cv.use_id(MR24HPC1PFComponent),
    cv.Optional(CONF_RESTART): button.button_schema(
        RestartButton,
        device_class=DEVICE_CLASS_RESTART,
        entity_category=ENTITY_CATEGORY_CONFIG,
        icon=ICON_RESTART_ALERT,
    ),
}


async def to_code(config):
    mr24hpc1_component = await cg.get_variable(config[CONF_MR24HPC1PF_ID])
    if restart_config := config.get(CONF_RESTART):
        b = await button.new_button(restart_config)
        await cg.register_parented(b, config[CONF_MR24HPC1PF_ID])
        cg.add(mr24hpc1_component.set_restart_button(b))
