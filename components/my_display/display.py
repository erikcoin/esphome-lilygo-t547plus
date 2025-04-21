import esphome.codegen as cg
import esphome.config_validation as cv
import esphome.components.display as display
from esphome.const import CONF_ID

my_display_ns = cg.esphome_ns.namespace('my_display')
MyEpaperDisplay = my_display_ns.class_('MyEpaperDisplay', cg.PollingComponent, display.DisplayBuffer)

CONFIG_SCHEMA = display.BASIC_DISPLAY_SCHEMA.extend({}).extend(
    cv.polling_component_schema("5s")  # kan ook "0s" zijn als je geen update-cyclus wil
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    #await cg.register_component(var, config)
    await display.register_display(var, config)
