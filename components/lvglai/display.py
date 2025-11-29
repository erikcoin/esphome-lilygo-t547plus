import esphome.codegen as cg
import esphome.components.display as display
from esphome.components import text_sensor
# Corrected import for automation helpers:
from esphome.automation import validate_automation, build_automation
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    CONF_LAMBDA,
    CONF_ROTATION,
    CONF_UPDATE_INTERVAL,
    CONF_WIDTH,
    CONF_HEIGHT,
)

DEPENDENCIES = ["network"]
AUTO_LOAD = ["display", "text_sensor", "binary_sensor"]

m5papers3_display_m5gfx_ns = cg.esphome_ns.namespace("m5papers3_display_m5gfx")

M5PaperS3DisplayM5GFX = m5papers3_display_m5gfx_ns.class_(
    "M5PaperS3DisplayM5GFX", cg.Component, display.Display
)

BUTTON_SCHEMA = cv.Schema({

    cv.Required(CONF_WIDTH): cv.int_range(min=1),
    cv.Required(CONF_HEIGHT): cv.int_range(min=1),

})

CONFIG_SCHEMA = cv.All(
    display.FULL_DISPLAY_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(M5PaperS3DisplayM5GFX),

        }
    ).extend(cv.COMPONENT_SCHEMA),
)


async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await display.register_display(var, config)
    
    if CONF_LAMBDA in config:
        lambda_ = await cg.process_lambda(
            config[CONF_LAMBDA], [(display.DisplayRef, "it")], return_type=cg.void
        )
        cg.add(var.set_writer(lambda_))

    if CONF_ROTATION in config:
        rotation_val = config[CONF_ROTATION]
        if isinstance(rotation_val, str):
            rotation_val = display.DISPLAY_ROTATIONS[rotation_val]
        cg.add(var.set_rotation(rotation_val))
        cg.add_define("USE_M5PAPER_S3_M5GFX")
