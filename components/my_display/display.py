import esphome.codegen as cg
#import esphome.components.display as display
from esphome.components import display
from esphome.const import CONF_ID
import esphome.config_validation as cv

my_display_ns = cg.esphome_ns.namespace("my_display22")
MyEpaperDisplay = my_display_ns.class_("MyEpaperDisplay", cg.Component, display.DisplayBuffer)

CONFIG_SCHEMA = display.BASIC_DISPLAY_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(MyEpaperDisplay),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
   
    var = cg.new_Pvariable(config[CONF_ID])
    await display.register_display(var, config)

    cg.add_build_flag("-DBOARD_HAS_PSRAM")
    cg.add_library("epdiy", repository="https://github.com/erikcoin/epdiy", version="None")
    cg.add_library("m5gfx", repository="https://github.com/erikcoin/M5GFX", version="None")
