import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import CONF_ID

DEPENDENCIES = ['sensor']

bmi270_ns = cg.esphome_ns.namespace('bmi270_sensor')
BMI270Sensor = bmi270_ns.class_('BMI270Sensor', cg.PollingComponent)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(BMI270Sensor),

    cv.Optional("accel_x"): sensor.sensor_schema(
        unit_of_measurement="g",
        icon="mdi:axis-x-arrow",
        accuracy_decimals=3
    ),
    cv.Optional("accel_y"): sensor.sensor_schema(
        unit_of_measurement="g",
        icon="mdi:axis-y-arrow",
        accuracy_decimals=3
    ),
    cv.Optional("accel_z"): sensor.sensor_schema(
        unit_of_measurement="g",
        icon="mdi:axis-z-arrow",
        accuracy_decimals=3
    ),
    cv.Optional("gyro_x"): sensor.sensor_schema(
        unit_of_measurement="°/s",
        icon="mdi:rotate-right",
        accuracy_decimals=1
    ),
    cv.Optional("gyro_y"): sensor.sensor_schema(
        unit_of_measurement="°/s",
        icon="mdi:rotate-right",
        accuracy_decimals=1
    ),
    cv.Optional("gyro_z"): sensor.sensor_schema(
        unit_of_measurement="°/s",
        icon="mdi:rotate-right",
        accuracy_decimals=1
    ),
}).extend(cv.polling_component_schema("1s"))

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)

    for axis in ['x', 'y', 'z']:
        if f"accel_{axis}" in config:
            sens = await sensor.new_sensor(config[f"accel_{axis}"])
            cg.add(getattr(var, f"accel_{axis}_sensor").set(sens))
        if f"gyro_{axis}" in config:
            sens = await sensor.new_sensor(config[f"gyro_{axis}"])
            cg.add(getattr(var, f"gyro_{axis}_sensor").set(sens))
