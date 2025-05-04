# my_components/m5papers3_display_m5gfx/display.py
import esphome.codegen as cg
import esphome.components.display as display
from esphome.components import text_sensor
import esphome.config_validation as cv
from esphome import automation
from esphome.const import (
    CONF_ID,
    CONF_LAMBDA,
    CONF_ROTATION,
    CONF_UPDATE_INTERVAL,
    #CONF_BRIGHTNESS # If you wanted to control overall brightness later
)

CONF_TOUCH_SENSOR = "touch_coordinates"

# Ensure necessary dependencies are declared for the build system
# AUTO_LOAD = ["display", "text_sensor"] # text_sensor might be needed if not implicitly loaded
DEPENDENCIES = ["network"] # Example, M5Unified might need network for some init? Check library deps.

# Namespace for our C++ code
m5papers3_display_m5gfx_ns = cg.esphome_ns.namespace("m5papers3_display_m5gfx")

# Define our C++ class, inheriting from DisplayBuffer now
M5PaperS3DisplayM5GFX = m5papers3_display_m5gfx_ns.class_(
    "M5PaperS3DisplayM5GFX", cg.Component, display.DisplayBuffer # Inherit from DisplayBuffer
)

CONFIG_SCHEMA = cv.All(
    # Use DISPLAY_SCHEMA which includes basic display props like rotation, update_interval
    display.FULL_DISPLAY_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(M5PaperS3DisplayM5GFX),
            cv.Optional(CONF_TOUCH_SENSOR): cv.use_id(text_sensor.TextSensor),
            # Add other config options if needed
        }
    ).extend(cv.COMPONENT_SCHEMA),
    cv.require_framework_version(esp_idf=cv.Version(4, 4, 0)), # M5Unified often needs specific IDF
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config) # Register as a component
    await display.register_display(var, config) # Register as a display

    # Add required libraries
    # Ensure these versions are compatible with your ESPHome version and each other
    cg.add_library("m5stack/M5Unified", ">=0.1.13") # Specify a version if known good
    cg.add_library("lovyan03/LovyanGFX", ">=1.1.9")  # M5GFX depends on LovyanGFX

    # Process lambda first if present
    if CONF_LAMBDA in config:
        lambda_ = await cg.process_lambda(
            config[CONF_LAMBDA], [(display.DisplayRef, "it")], return_type=cg.void
        )
        cg.add(var.set_writer(lambda_))

    # Set rotation if configured
    if CONF_ROTATION in config:
        cg.add(var.set_rotation(config[CONF_ROTATION]))

    # Set touch sensor if configured
    if CONF_TOUCH_SENSOR in config:
        touch_sensor = await cg.get_variable(config[CONF_TOUCH_SENSOR])
        cg.add(var.set_touch_sensor(touch_sensor))

    # Optional: If you expose brightness control
    # if CONF_BRIGHTNESS in config:
    #    cg.add(var.set_brightness(config[CONF_BRIGHTNESS]))

    # No specific code needed here for grayscale - it's handled in C++ setup
    cg.add_define("USE_M5PAPER_S3_M5GFX") # Optional: Define for conditional compilation if needed elsewhere
