
import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import display
from esphome.const import CONF_ID

CODEOWNERS = ["@your-github-username"]  # Optional
DEPENDENCIES = ["display"]

my_display_ns = cg.esphome_ns.namespace("my_display")


MyDisplay = my_display_ns.class_(
    "MyDisplay", cg.PollingComponent, display.DisplayBuffer
)
CONFIG_SCHEMA = display.BASIC_DISPLAY_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(MyDisplay),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await display.register_display(var, config)
