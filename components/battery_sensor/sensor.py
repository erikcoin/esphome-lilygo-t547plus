import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    CONF_ID,
    UNIT_VOLT,
    ICON_BATTERY,
    DEVICE_CLASS_VOLTAGE,
    CONF_UPDATE_INTERVAL,
)

my_battery_ns = cg.esphome_ns.namespace('battery_sensor')
MyBatterySensor = my_battery_ns.class_('MyBatterySensor', sensor.Sensor, cg.PollingComponent)
CONFIG_SCHEMA = sensor.sensor_schema(
    unit_of_measurement=UNIT_VOLT,
    icon=ICON_BATTERY,
    accuracy_decimals=2,
    device_class=DEVICE_CLASS_VOLTAGE,
).extend({
#CONFIG_SCHEMA = sensor.sensor_schema(UNIT_VOLT, ICON_BATTERY, 2, DEVICE_CLASS_VOLTAGE).extend({
    cv.GenerateID(): cv.declare_id(MyBatterySensor),
    cv.Optional(CONF_UPDATE_INTERVAL, default='60s'): cv.update_interval,
})

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    #await sensor.register_sensor(var, config)
