# my_components/m5papers3_display_m5gfx/display.py
import esphome.codegen as cg
import esphome.components.display as display
from esphome.components import text_sensor
from esphome.components import sensor
import esphome.config_validation as cv
#from esphome.components import display
from esphome import automation
from esphome.const import (
    CONF_ID,
    CONF_LAMBDA,
    CONF_ROTATION,
    CONF_UPDATE_INTERVAL,
)
CONF_TOUCH_SENSOR = "touch_coordinates"
CONF_TOUCH_X_SENSOR = "touch_x"
CONF_TOUCH_Y_SENSOR = "touch_y"
DEPENDENCIES = ["sensor"]
AUTO_LOAD = ["display"]
# Namespace voor onze C++ code
# Gebruik een andere namespace om conflicten te vermijden als je beide hebt
m5papers3_display_m5gfx_ns = cg.esphome_ns.namespace("m5papers3_display_m5gfx")
# Definieer onze C++ class
M5PaperS3DisplayM5GFX = m5papers3_display_m5gfx_ns.class_(
    "M5PaperS3DisplayM5GFX", cg.Component, display.Display
)

CONFIG_SCHEMA = display.FULL_DISPLAY_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(M5PaperS3DisplayM5GFX),
        cv.Optional(CONF_TOUCH_SENSOR): cv.use_id(text_sensor.TextSensor),
        cv.Optional(CONF_TOUCH_X_SENSOR): cv.use_id(sensor.Sensor),
        cv.Optional(CONF_TOUCH_Y_SENSOR): cv.use_id(sensor.Sensor),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    #await cg.register_component(var, config)
    await display.register_display(var, config)

    # Voeg M5GFX en M5Unified libraries toe
    # M5Unified is nodig voor M5.begin() die M5.Display configureert
   # cg.add_library("m5stack/M5Unified", None)
   # cg.add_library("m5stack/M5GFX", None)
    if CONF_TOUCH_SENSOR in config:
        touch_sensor = await cg.get_variable(config[CONF_TOUCH_SENSOR])
        cg.add(var.set_touch_sensor(touch_sensor))
    
    if CONF_LAMBDA in config:
        lambda_ = await cg.process_lambda(
            config[CONF_LAMBDA], [(display.DisplayRef, "it")], return_type=cg.void
        )
        cg.add(var.set_writer(lambda_))

    if CONF_ROTATION in config:
        cg.add(var.set_rotation(config[CONF_ROTATION]))

    if CONF_UPDATE_INTERVAL in config:
        cg.add(var.set_update_interval(config[CONF_UPDATE_INTERVAL]))

    if CONF_TOUCH_SENSOR in config:
        touch_sensor = await cg.get_variable(config[CONF_TOUCH_SENSOR])
        cg.add(var.set_touch_sensor(touch_sensor))
    
    #if CONF_TOUCH_X_SENSOR in config:
    #    touch_x = await cg.get_variable(config[CONF_TOUCH_X_SENSOR])
    #    cg.add(var.set_touch_x_sensor(touch_x))

    #if CONF_TOUCH_Y_SENSOR in config:
    #    touch_y = await cg.get_variable(config[CONF_TOUCH_Y_SENSOR])
    #    cg.add(var.set_touch_y_sensor(touch_y))
    if CONF_TOUCH_Y_SENSOR in config:
        sens = await sensor.new_sensor(config["touch_x"])
        cg.call(mydisplay.set_touch_x_sensor)(sens)

    if CONF_TOUCH_Y_SENSOR in config:
        sens = await sensor.new_sensor(config["touch_y"])
        cg.call(mydisplay.set_touch_y_sensor)(sens)
