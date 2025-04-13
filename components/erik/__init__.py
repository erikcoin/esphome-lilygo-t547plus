from esphome import automation
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import display, text_sensor
from esphome.const import CONF_ID

erik_ns = cg.esphome_ns.namespace("erik")
ErikDisplay = erik_ns.class_("ErikDisplay", display.DisplayBuffer)

CONF_LIGHT_STATE = "light_state"

CONFIG_SCHEMA = display.BASIC_DISPLAY_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(ErikDisplay),
    cv.Required(CONF_LIGHT_STATE): cv.use_id(text_sensor.TextSensor),
}).extend(cv.polling_component_schema("1s"))

def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield display.register_display(var, config)
    yield cg.register_component(var, config)

    light_state = yield cg.get_variable(config[CONF_LIGHT_STATE])
    cg.add(var.set_light_state(light_state))
