import esphome.codegen as cg
import esphome.config_validation as cv
import esphome.components.display as display
from esphome.const import CONF_ID

my_display_ns = cg.esphome_ns.namespace('my_display')
MyEpaperDisplay = my_display_ns.class_('MyEpaperDisplay', cg.PollingComponent, display.DisplayBuffer)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(MyEpaperDisplay),
}).extend(display.BASIC_DISPLAY_SCHEMA).extend(
    cv.polling_component_schema("5s")
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await display.register_display(var, config)
