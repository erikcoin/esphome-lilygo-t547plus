import esphome.codegen as cg
import esphome.config_validation as cv
from esphome import core
from esphome.components import lvgl
from esphome.const import CONF_ID

DEPENDENCIES = ['lvgl']

m5papers3_lvgl_ns = cg.esphome_ns.namespace('m5papers3_lvgl')
M5PaperS3LVGL = m5papers3_lvgl_ns.class_('M5PaperS3LVGL', cg.Component)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(M5PaperS3LVGL),
}).extend(cv.COMPONENT_SCHEMA)

def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)
