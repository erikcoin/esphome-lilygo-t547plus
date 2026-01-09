
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID

ram_logger_ns = cg.esphome_ns.namespace("ram_logger")
RamLogger = ram_logger_ns.class_("RamLogger", cg.Component)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(RamLogger),
})

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
