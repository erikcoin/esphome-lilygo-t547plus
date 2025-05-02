import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    CONF_ID,
    CONF_NAME,
    UNIT_VOLT,
    UNIT_PERCENT,
    ICON_BATTERY,
    DEVICE_CLASS_VOLTAGE,
    DEVICE_CLASS_BATTERY,
    CONF_UPDATE_INTERVAL,
)

battery_ns = cg.esphome_ns.namespace('battery_sensor')
MyBatterySensor = battery_ns.class_('MyBatterySensor', sensor.Sensor, cg.PollingComponent)
MyBatteryPercentageSensor = battery_ns.class_('MyBatteryPercentageSensor', sensor.Sensor, cg.PollingComponent)

CONF_PERCENTAGE = "percentage"

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(MyBatterySensor),
    cv.Optional(CONF_NAME, default="Battery Voltage"): cv.string,
    cv.Optional(CONF_PERCENTAGE): cv.declare_id(MyBatteryPercentageSensor),
    cv.Optional(CONF_UPDATE_INTERVAL, default="60s"): cv.update_interval,
})

async def to_code(config):
    voltage = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(voltage, config)
    await sensor.register_sensor(voltage, config)


    if CONF_PERCENTAGE in config:
        percent = cg.new_Pvariable(config[CONF_PERCENTAGE])
        await cg.register_component(percent, config)
        await sensor.register_sensor(percent, config)

