import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.components import display

# Gebruik Component in plaats van een namespace
Papertouch = cg.Component

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(Papertouch),
    cv.Required("display"): cv.use_id(display.Display),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):

    #var = cg.new_Pvariable(config[CONF_ID])
    #await cg.register_component(var, config)
    #await display.register_display(var, config)


    
    var = await display.register_display(var, config)
    #var = await cg.register_component(config, Papertouch.new())
    display_var = await cg.get_variable(config["display"])
    cg.add(var.set_display(display_var))
    #await display.register_display(var, config)
