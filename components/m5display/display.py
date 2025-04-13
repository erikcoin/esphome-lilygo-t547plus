import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import core
from esphome.components.display import DisplayBuffer, DisplayElement

# Import the C++ class (this should match the C++ class name in your m5paper_display.cpp)
from esphome.components.m5display import M5PaperDisplay

# Define a new namespace for this component
m5display_ns = cg.esphome_ns.namespace("m5display")
M5PaperDisplay = m5display_ns.class_("M5PaperDisplay", cg.PollingComponent)

# The `display` function that will be exposed in the YAML
def to_code(config):
    # The lambda in YAML will create a new instance of M5PaperDisplay
    var = cg.new_Pvariable(config[core.ConfItem("id")], config["id"])

    # Register this as a component
    cg.add(var.set_name(config["id"]))
    cg.add_app_component(var)

    # We can specify settings for the display (like update_interval, etc)
    cg.add(var.set_update_interval(config["update_interval"]))

    return var


# Define the schema for the component
CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(M5PaperDisplay),  # Declare the component id
    cv.Required("id"): cv.use_id(core.ID),
    cv.Optional("update_interval", default=1000): cv.positive_int,
}).extend(core.COMPONENT_SCHEMA)
