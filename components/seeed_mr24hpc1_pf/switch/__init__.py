import esphome.codegen as cg
from esphome.components import switch
import esphome.config_validation as cv
from esphome.const import (
    DEVICE_CLASS_SWITCH,
    ENTITY_CATEGORY_CONFIG,
)
from .. import CONF_MR24HPC1PF_ID, MR24HPC1PFComponent, mr24hpc1pf_ns

UnderlyingOpenFuncSwitch = mr24hpc1pf_ns.class_(
    "UnderlyOpenFunctionSwitch", switch.Switch
)

CONF_UNDERLYING_OPEN_FUNCTION = "underlying_open_function"

CONFIG_SCHEMA = {
    cv.GenerateID(CONF_MR24HPC1PF_ID): cv.use_id(MR24HPC1PFComponent),
}


async def to_code(config):
    mr24hpc1pf_component = await cg.get_variable(config[CONF_MR24HPC1PF_ID])