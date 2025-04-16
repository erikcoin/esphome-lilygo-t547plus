import esphome.codegen as cg
import esphome.components.display as display
from esphome.components import font
import esphome.config_validation as cv
from esphome.const import CONF_ID

my_epaper_ns = cg.esphome_ns.namespace("my_epaper_display")
MyEpaperDisplay = my_epaper_ns.class_("MyEpaperDisplay", cg.Component, display.DisplayBuffer)

CONFIG_SCHEMA = display.BASIC_DISPLAY_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(MyEpaperDisplay),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await display.register_display(var, config)
