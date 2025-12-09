import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import automation
from esphome.const import CONF_ID

AUTO_LOAD = ["esp32", "logger"]

m5p3_ns = cg.esphome_ns.namespace("m5p3_light_sleep")
M5P3LightSleepComponent = m5p3_ns.class_("M5P3LightSleep", cg.Component)

CONF_WAKE_ON_TOUCH = "wake_on_touch"
CONF_TURN_OFF_DISPLAY = "turn_off_display"
CONF_MIN_INACTIVE_TIME = "min_inactive_time"
CONF_VERBOSE = "verbose"
CONF_WAKE_EVERY = "wake_every"

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(): cv.declare_id(M5P3LightSleepComponent),
        cv.Optional(CONF_WAKE_ON_TOUCH, default=True): cv.boolean,
        cv.Optional(CONF_TURN_OFF_DISPLAY, default=True): cv.boolean,
        cv.Optional(CONF_MIN_INACTIVE_TIME, default="30s"): cv.positive_time_period_milliseconds,
        cv.Optional(CONF_VERBOSE, default=False): cv.boolean,
        cv.Optional(CONF_WAKE_EVERY, default="0s"): cv.positive_time_period_milliseconds,

        cv.Optional("on_wake"): automation.validate_automation({
            cv.GenerateID(): cv.declare_id(automation.Trigger)
        }),
    }
).extend(cv.COMPONENT_SCHEMA)


# ACTION REGISTERED SAME AS BEFORE…

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])

    cg.add(var.set_wake_on_touch(config[CONF_WAKE_ON_TOUCH]))
    cg.add(var.set_turn_off_display(config[CONF_TURN_OFF_DISPLAY]))
    cg.add(var.set_verbose(config[CONF_VERBOSE]))
    cg.add(var.set_min_inactive_time(config[CONF_MIN_INACTIVE_TIME]))
    cg.add(var.set_wake_every(config[CONF_WAKE_EVERY]))

    await cg.register_component(var, config)

    if "on_wake" in config:
        for conf in config["on_wake"]:
            trigger = var.get_on_wake_trigger()
            await automation.build_automation(trigger, conf)
