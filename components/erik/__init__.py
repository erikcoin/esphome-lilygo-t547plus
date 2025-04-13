import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import light
from esphome.const import CONF_ID

erik_ns = cg.esphome_ns.namespace("erik")
ErikComponent = erik_ns.class_("ErikComponent", cg.PollingComponent)

CONF_LIGHT = "light"

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(ErikComponent),
    cv.Required(CONF_LIGHT): cv.use_id(light.LightState),
}).extend(cv.polling_component_schema("1s"))

def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)

    lt = yield cg.get_variable(config[CONF_LIGHT])
    cg.add(var.set_light(lt))
