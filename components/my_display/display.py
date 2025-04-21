import esphome.codegen as cg
import esphome.components.display as display
from esphome import automation

my_display_ns = cg.esphome_ns.namespace('my_display')
MyEpaperDisplay = my_display_ns.class_('MyEpaperDisplay', cg.PollingComponent, display.DisplayBuffer)

CONFIG_SCHEMA = display.BASIC_DISPLAY_SCHEMA.extend({}).extend(
    cg.polling_component_schema('5s')  # of 0s als je alleen init wil
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await display.register_display(var, config)
