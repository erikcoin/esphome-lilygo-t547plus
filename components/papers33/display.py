# my_components/m5papers3_display_m5gfx/display.py
import esphome.codegen as cg
import esphome.components.display as display
from esphome.components import text_sensor
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    CONF_LAMBDA,
    CONF_ROTATION,
    CONF_UPDATE_INTERVAL,
)

CONF_TOUCH_SENSOR = "touch_coordinates"
DEPENDENCIES = ["network"] # Optional: Add if M5Unified needs it
AUTO_LOAD = ["display", "text_sensor"] # Ensure text_sensor component is loaded

m5papers3_display_m5gfx_ns = cg.esphome_ns.namespace("m5papers3_display_m5gfx")

# *** Ensure inheritance is from display.Display ***
M5PaperS3DisplayM5GFX = m5papers3_display_m5gfx_ns.class_(
    "M5PaperS3DisplayM5GFX", cg.Component, display.Display # Base class is display.Display
)

CONFIG_SCHEMA = cv.All(
    # Use base Display schema, not FULL_DISPLAY_SCHEMA if not using DisplayBuffer features directly
    display.DULL_DISPLAY_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(M5PaperS3DisplayM5GFX),
            cv.Optional(CONF_LAMBDA): cv.returning_lambda, # Standard lambda config
            cv.Optional(CONF_ROTATION): cv.int_range(min=0, max=360) | cv.enum(display.DISPLAY_ROTATIONS, upper=True), # Allow degrees or enum
            cv.Optional(CONF_UPDATE_INTERVAL): cv.update_interval, # Standard update interval
            cv.Optional(CONF_TOUCH_SENSOR): cv.use_id(text_sensor.TextSensor),
        }
    ).extend(cv.COMPONENT_SCHEMA),
    #cv.require_framework_version(esp_idf=cv.Version(4, 4, 0)), # Example IDF version
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    # *** Register using display.register_display ***
    await display.register_display(var, config)

    # Add libraries
    #cg.add_library("m5stack/M5Unified", ">=0.1.13")
    #cg.add_library("lovyan03/LovyanGFX", ">=1.1.9")

    # Process lambda
    if CONF_LAMBDA in config:
        lambda_ = await cg.process_lambda(
            config[CONF_LAMBDA], [(display.DisplayRef, "it")], return_type=cg.void
        )
        cg.add(var.set_writer(lambda_)) # Use correct setter name

    # Process rotation - needs conversion from degrees/enum to 0,1,2,3
    if CONF_ROTATION in config:
        rotation_val = config[CONF_ROTATION]
        if isinstance(rotation_val, str): # If using enum like '90°'
             rotation_val = display.DISPLAY_ROTATIONS[rotation_val] # Get degrees
        # Convert degrees (0, 90, 180, 270) to M5GFX rotation (0, 1, 2, 3)
        # The C++ set_rotation method already handles this conversion
        cg.add(var.set_rotation(rotation_val))


    # Update interval is handled by Component registration automatically

    # Set touch sensor
    if CONF_TOUCH_SENSOR in config:
        touch_sensor = await cg.get_variable(config[CONF_TOUCH_SENSOR])
        cg.add(var.set_touch_sensor(touch_sensor))

    cg.add_define("USE_M5PAPER_S3_M5GFX") # Optional define
