import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import display
from esphome.const import CONF_ID

erik_ns = cg.esphome_ns.namespace("erik")
ErikDisplay = erik_ns.class_("ErikDisplay", display.DisplayBuffer)

CONF_ON_TOUCH = "on_touch"

CONFIG_SCHEMA = display.BASIC_DISPLAY_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(ErikDisplay),
    cv.Optional(CONF_ON_TOUCH): cv.returning_lambda,
}).extend(cv.polling_component_schema("1s"))

def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield display.register_display(var, config)

    if touch_lambda := config.get(CONF_ON_TOUCH):
        cg.add(var.set_touch_callback(touch_lambda))
