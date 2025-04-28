import esphome.codegen as cg
import esphome.config_validation as cv
import esphome.components.touchscreen as touchscreen

m5touch_ns = cg.esphome_ns.namespace('m5touch')
M5TouchComponent = m5touch_ns.class_('M5TouchComponent', touchscreen.Touchscreen, cg.Component)

CONFIG_SCHEMA = touchscreen.TOUCHSCREEN_SCHEMA.extend({}).extend(
    cv.COMPONENT_SCHEMA
)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    #await cg.register_component(var, config)
    await touchscreen.register_touchscreen(var, config)
