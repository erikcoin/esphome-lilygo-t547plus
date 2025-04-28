import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import touchscreen
from esphome.const import CONF_ID

# Define the namespace where this component will live
papertouch_ns = cg.esphome_ns.namespace('papertouch')

# Link the C++ class (defined in the header) with the Python code
Papertouch = papertouch_ns.class_('Papertouch', touchscreen.Touchscreen, cg.Component)

# Define the configuration schema for the custom component
CONFIG_SCHEMA = touchscreen.TOUCHSCREEN_SCHEMA.extend({
    cv.Required(CONF_ID): cv.declare_id(Papertouch),
}).extend(cv.COMPONENT_SCHEMA)

# This function converts the configuration into C++ code
async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    # Register the component in ESPHome
    await cg.register_component(var, config)
    # Register the touchscreen in ESPHome (to handle touch events)
    await touchscreen.register_touchscreen(var, config)
