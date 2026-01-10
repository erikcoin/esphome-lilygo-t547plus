import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID
from esphome import pins

DEPENDENCIES = ["esp32"]
CODEOWNERS = ["@custom"]

light_sleep_ns = cg.esphome_ns.namespace("light_sleep")
LightSleepComponent = light_sleep_ns.class_("LightSleepComponent", cg.Component)

CONF_SLEEP_DURATION = "sleep_duration"
CONF_RUN_DURATION = "run_duration"
CONF_WAKEUP_PIN = "wakeup_pin"
CONF_WAKEUP_LEVEL = "wakeup_level"

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(LightSleepComponent),
    cv.Optional(CONF_SLEEP_DURATION, default="5min"): cv.positive_time_period_milliseconds,
    cv.Optional(CONF_RUN_DURATION, default="1min"): cv.positive_time_period_milliseconds,
    cv.Optional(CONF_WAKEUP_PIN, default=48): pins.internal_gpio_input_pin_schema,
    cv.Optional(CONF_WAKEUP_LEVEL, default=0): cv.int_range(min=0, max=1),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    
    cg.add(var.set_sleep_duration(config[CONF_SLEEP_DURATION]))
    cg.add(var.set_run_duration(config[CONF_RUN_DURATION]))
    
    if CONF_WAKEUP_PIN in config:
        pin = await cg.gpio_pin_expression(config[CONF_WAKEUP_PIN])
        cg.add(var.set_wakeup_pin(pin))
        cg.add(var.set_wakeup_level(config[CONF_WAKEUP_LEVEL]))
