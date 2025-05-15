import esphome.codegen as cg
import esphome.components.display as display
from esphome.components import text_sensor, automation # <-- Import automation
import esphome.config_validation as cv
from esphome.const import (
    CONF_ID,
    CONF_LAMBDA,
    CONF_ROTATION,
    CONF_UPDATE_INTERVAL,
    CONF_X, # Standard coordinate keys
    CONF_Y,
    CONF_WIDTH,
    CONF_HEIGHT,
    CONF_ON_PRESS,
    # CONF_ON_RELEASE, # You could add this later
)

CONF_TOUCH_SENSOR = "touch_coordinates"
CONF_BUTTONS = "buttons" # New key for button configuration

DEPENDENCIES = ["network"]
AUTO_LOAD = ["display", "text_sensor", "binary_sensor"] # binary_sensor might be useful if buttons expose state

m5papers3_display_m5gfx_ns = cg.esphome_ns.namespace("m5papers3_display_m5gfx")

M5PaperS3DisplayM5GFX = m5papers3_display_m5gfx_ns.class_(
    "M5PaperS3DisplayM5GFX", cg.Component, display.Display
)

# Schema for a single button
BUTTON_SCHEMA = cv.Schema({
    cv.Required(CONF_X): cv.int_range(min=0),
    cv.Required(CONF_Y): cv.int_range(min=0),
    cv.Required(CONF_WIDTH): cv.int_range(min=1),
    cv.Required(CONF_HEIGHT): cv.int_range(min=1),
    cv.Optional(CONF_ON_PRESS): automation.validate_automation(single=True),
    # cv.Optional(CONF_ON_RELEASE): automation.validate_automation(single=True), # For future
    # cv.Optional(CONF_ID): cv.declare_id(binary_sensor.BinarySensor), # If you want buttons to be binary_sensors
})

CONFIG_SCHEMA = cv.All(
    display.FULL_DISPLAY_SCHEMA.extend(
        {
            cv.GenerateID(): cv.declare_id(M5PaperS3DisplayM5GFX),
            cv.Optional(CONF_TOUCH_SENSOR): cv.use_id(text_sensor.TextSensor),
            cv.Optional(CONF_BUTTONS): cv.ensure_list(BUTTON_SCHEMA), # List of buttons
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

    # Process buttons
    if CONF_BUTTONS in config:
        for i, button_config in enumerate(config[CONF_BUTTONS]):
            # Create an automation for on_press
            on_press_automation = None
            if CONF_ON_PRESS in button_config:
                # The trigger type for the automation will be void (no parameters from C++ to automation)
                # The Automation object itself is what we need to pass to C++
                # We'll need to create a unique ID for the automation storage in C++
                # ESPHome handles the actual creation of the Automation object internally
                # when build_automation is called.
                # We'll create a C++ Automation<> object that the display component can trigger.
                auto = cg.new_Pvariable(button_config[CONF_ON_PRESS]) # This is the trigger's config ID
                await automation.build_automation(
                    auto, # The Automation object
                    [],   # Parameters for the lambda in the automation (empty for simple trigger)
                    button_config[CONF_ON_PRESS] # The automation configuration from YAML
                )
                on_press_automation = auto # Store the pointer to the Automation object

            # Add the button to the C++ component
            # We pass nullptr if no automation is configured
            cg.add(var.add_button(
                button_config[CONF_X],
                button_config[CONF_Y],
                button_config[CONF_WIDTH],
                button_config[CONF_HEIGHT],
                on_press_automation if on_press_automation else cg.nullptr # Pass the automation obj or nullptr
            ))

    cg.add_define("USE_M5PAPER_S3_M5GFX")
