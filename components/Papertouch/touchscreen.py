import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import touchscreen
from esphome.const import CONF_ID

papertouch_ns = cg.esphome_ns.namespace('papertouch')
PaperTouch = papertouch_ns.class_('PaperTouch', touchscreen.Touchscreen, cg.Component)


CONFIG_SCHEMA = touchscreen.TOUCHSCREEN_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(PaperTouch),
    }
).extend(cv.COMPONENT_SCHEMA)
async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    #await cg.register_component(var, config)
    await touchscreen.register_touchscreen(var, config)
