import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import touchscreen
from esphome.components import text_sensor
from esphome.const import CONF_ID

CONF_TOUCH_COORDINATES = "touch_coordinates"

papertouch_ns = cg.esphome_ns.namespace('papertouch')
PaperTouch = papertouch_ns.class_('PaperTouch', touchscreen.Touchscreen, cg.Component)

CONFIG_SCHEMA = touchscreen.TOUCHSCREEN_SCHEMA.extend(
    {
        cv.GenerateID(): cv.declare_id(PaperTouch),
        cv.Required(CONF_TOUCH_COORDINATES): cv.use_id(text_sensor.TextSensor),  # <--- hier toegevoegd
    }
).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await touchscreen.register_touchscreen(var, config)

    # Touch coordinates sensor koppelen
    touch_sensor = await cg.get_variable(config[CONF_TOUCH_COORDINATES])
    cg.add(var.set_touch_sensor(touch_sensor))  # <--- hier toegevoegd
