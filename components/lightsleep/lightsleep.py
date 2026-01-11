import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_WAKEUP_PIN

lightsleep_ns = cg.esphome_ns.namespace("lightsleep")
LightSleepComponent = lightsleep_ns.class_("LightSleepComponent", cg.Component)

CONF_WAKE_ON_TOUCH = "wake_on_touch"
CONF_TURN_OFF_DISPLAY = "turn_off_display"
CONF_MIN_INACTIVE_TIME = "min_inactive_time"
CONF_WAKE_EVERY = "wake_every"

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(LightSleepComponent),

    cv.Optional(CONF_WAKEUP_PIN): cv.int_,
    cv.Optional(CONF_WAKE_ON_TOUCH, default=True): cv.boolean,
    cv.Optional(CONF_TURN_OFF_DISPLAY, default=False): cv.boolean,

    # Duration types (seconds, ms, etc.)
    cv.Optional(CONF_MIN_INACTIVE_TIME, default="30s"): cv.positive_time_period_milliseconds,
    cv.Optional(CONF_WAKE_EVERY, default="0s"): cv.positive_time_period_milliseconds,
})

def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)

    if CONF_WAKEUP_PIN in config:
        cg.add(var.set_wakeup_pin(config[CONF_WAKEUP_PIN]))

    cg.add(var.set_wake_on_touch(config[CONF_WAKE_ON_TOUCH]))
    cg.add(var.set_turn_off_display(config[CONF_TURN_OFF_DISPLAY]))

    cg.add(var.set_min_inactive_time(config[CONF_MIN_INACTIVE_TIME].total_milliseconds))
    cg.add(var.set_wake_every(config[CONF_WAKE_EVERY].total_milliseconds))
