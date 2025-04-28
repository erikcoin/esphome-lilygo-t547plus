import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.components import display, touchscreen

Papertouch = cg.Component

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(Papertouch),
    cv.Required("display"): cv.use_id(display.Display),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    # Registreer de component
    var = cg.new_Papertouch()  # Dit moet het component correct instantiëren
    await cg.register_component(var, config)

    # Haal de display variable op en koppel deze aan het touchscreen
    display_var = await cg.get_variable(config["display"])
    var.set_display(display_var)  # Koppel de display aan het touchscreen component
