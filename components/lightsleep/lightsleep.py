import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.const import CONF_ID

lightsleep_ns = cg.esphome_ns.namespace("lightsleep")
LightSleepComponent = lightsleep_ns.class_(
    "LightSleepComponent", cg.Component
)

CONF_WAKEUP_PIN = "wakeup_pin"
CONF_WAKEUP_MINUTES = "wakeup_minutes"

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(LightSleepComponent),

    cv.Optional(CONF_WAKEUP_PIN): cv.int_,
    cv.Optional(CONF_WAKEUP_MINUTES, default=0): cv.All(cv.int_, cv.Range(min=0)),
})

def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)

    if CONF_WAKEUP_PIN in config:
        cg.add(var.set_wakeup_pin(config[CONF_WAKEUP_PIN]))

    cg.add(var.set_wakeup_minutes(config.get(CONF_WAKEUP_MINUTES, 0)))
