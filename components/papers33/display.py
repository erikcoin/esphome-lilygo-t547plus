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
    CONF_X_GRID,
    CONF_Y_GRID,
    CONF_WIDTH,
    CONF_HEIGHT,
    CONF_ON_PRESS,
)

CONF_TOUCH_SENSOR = "touch_coordinates"
CONF_BUTTONS = "buttons"
CONF_BUTTON_ID = "buttonid"

DEPENDENCIES = ["network"]
AUTO_LOAD = ["display", "text_sensor", "binary_sensor"]

m5papers3_display_m5gfx_ns = cg.esphome_ns.namespace("m5papers3_display_m5gfx")

M5PaperS3DisplayM5GFX = m5papers3_display_m5gfx_ns.class_(
    "M5PaperS3DisplayM5GFX", cg.Component, display.Display
)

BUTTON_SCHEMA = cv.Schema({
    cv.Required(CONF_X_GRID): cv.int_range(min=0),
    cv.Required(CONF_Y_GRID): cv.int_range(min=0),
    cv.Required(CONF_WIDTH): cv.int_range(min=1),
    cv.Required(CONF_HEIGHT): cv.int_range(min=1),
    cv.Required(CONF_BUTTON_ID): cv.string,
    # Use validate_automation directly
    cv.Optional(CONF_ON_PRESS): validate_automation(single=True),
})

CONFIG_SCHEMA = cv.All(
    display.FULL_DISPLAY_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(M5PaperS3DisplayM5GFX),
            cv.Optional(CONF_TOUCH_SENSOR): cv.use_id(text_sensor.TextSensor),
            cv.Optional(CONF_BUTTONS): cv.ensure_list(BUTTON_SCHEMA),
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

    if CONF_TOUCH_SENSOR in config:
        touch_sens = await cg.get_variable(config[CONF_TOUCH_SENSOR])
        cg.add(var.set_touch_sensor(touch_sens))

    if CONF_BUTTONS in config:
        for button_config in config[CONF_BUTTONS]:
            trigger = None
            if CONF_ON_PRESS in button_config:
                trigger = var.make_button_trigger(button_config[CONF_BUTTON_ID])
                await build_automation(trigger, [], button_config[CONF_ON_PRESS])

            cg.add(var.add_button(
                button_config[CONF_X_GRID],
                button_config[CONF_Y_GRID],
                button_config[CONF_WIDTH],
                button_config[CONF_HEIGHT],
                button_config[CONF_BUTTON_ID],
                trigger or cg.nullptr
            ))
            print("5Button trigger created:", trigger)

    

    cg.add_define("USE_M5PAPER_S3_M5GFX")
