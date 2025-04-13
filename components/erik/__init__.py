
import esphome.codegen as cg
import esphome.config_validation as cv

CONF_TEXT = "text"
CONF_X = "x"
CONF_Y = "y"
CONF_SIZE = "size"

erik_ns = cg.esphome_ns.namespace("erik")
ErikComponent = erik_ns.class_("ErikComponent", cg.Component)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(ErikComponent),
    cv.Required(CONF_TEXT): cv.string,
    cv.Optional(CONF_X, default=10): cv.int_,
    cv.Optional(CONF_Y, default=10): cv.int_,
    cv.Optional(CONF_SIZE, default=2): cv.int_,
}).extend(cv.COMPONENT_SCHEMA)

def to_code(config):
    var = cg.new_Pvariable(config[cv.GenerateID()])
    yield cg.register_component(var, config)
    cg.add(var.set_text(config[CONF_TEXT]))
    cg.add(var.set_x(config[CONF_X]))
    cg.add(var.set_y(config[CONF_Y]))
    cg.add(var.set_size(config[CONF_SIZE]))
