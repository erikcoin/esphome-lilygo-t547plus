import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import touchscreen
from esphome.components import display

Papertouch = touchscreen.TouchscreenComponent.namespace.class_('Papertouch', touchscreen.TouchscreenComponent)

CONFIG_SCHEMA = touchscreen.TOUCHSCREEN_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(Papertouch),
    cv.Required("display"): cv.use_id(display.Display),
})
  
async def to_code(config):
    var = await touchscreen.new_touchscreen_component(config)
    display_var = await cg.get_variable(config["display"])
    cg.add(var.set_display(display_var))
    await touchscreen.register_touchscreen(var, config)


