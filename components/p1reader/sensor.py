import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import sensor
from esphome.const import (
    CONF_ID,
    DEVICE_CLASS_CURRENT,
    DEVICE_CLASS_ENERGY,
    DEVICE_CLASS_POWER,
    DEVICE_CLASS_REACTIVE_ENERGY,
    DEVICE_CLASS_REACTIVE_POWER,
    DEVICE_CLASS_VOLTAGE,
    STATE_CLASS_MEASUREMENT,
    STATE_CLASS_TOTAL_INCREASING,
    UNIT_AMPERE,
    UNIT_KILOWATT,
    UNIT_KILOWATT_HOURS,
    UNIT_KILOVOLT_AMPS_REACTIVE_HOURS,
    UNIT_KILOVOLT_AMPS_REACTIVE,
    UNIT_VOLT,
)
from . import P1Reader, CONF_P1READER_ID

AUTO_LOAD = ["p1reader"]


def energy_schema():
    return sensor.sensor_schema(
        unit_of_measurement=UNIT_KILOWATT_HOURS,
        accuracy_decimals=3,
        device_class=DEVICE_CLASS_ENERGY,
        state_class=STATE_CLASS_TOTAL_INCREASING,
    )


def reactive_energy_schema():
    return sensor.sensor_schema(
        unit_of_measurement=UNIT_KILOVOLT_AMPS_REACTIVE_HOURS,
        accuracy_decimals=3,
        device_class=DEVICE_CLASS_REACTIVE_ENERGY,
        state_class=STATE_CLASS_TOTAL_INCREASING,
    )


def power_schema():
    return sensor.sensor_schema(
        unit_of_measurement=UNIT_KILOWATT,
        accuracy_decimals=3,
        device_class=DEVICE_CLASS_POWER,
        state_class=STATE_CLASS_MEASUREMENT,
    )


def reactive_power_schema():
    return sensor.sensor_schema(
        unit_of_measurement=UNIT_KILOVOLT_AMPS_REACTIVE,
        accuracy_decimals=3,
        device_class=DEVICE_CLASS_REACTIVE_POWER,
        state_class=STATE_CLASS_MEASUREMENT,
    )


def voltage_schema():
    return sensor.sensor_schema(
        unit_of_measurement=UNIT_VOLT,
        accuracy_decimals=3,
        device_class=DEVICE_CLASS_VOLTAGE,
        state_class=STATE_CLASS_MEASUREMENT,
    )


def current_schema():
    return sensor.sensor_schema(
        unit_of_measurement=UNIT_AMPERE,
        accuracy_decimals=3,
        device_class=DEVICE_CLASS_CURRENT,
        state_class=STATE_CLASS_MEASUREMENT,
    )


SENSOR_TYPES = {
    "cumulative_active_import": energy_schema,
    "cumulative_active_export": energy_schema,
    "cumulative_reactive_import": reactive_energy_schema,
    "cumulative_reactive_export": reactive_energy_schema,
    "momentary_active_import": power_schema,
    "momentary_active_export": power_schema,
    "momentary_reactive_import": reactive_power_schema,
    "momentary_reactive_export": reactive_power_schema,
    "momentary_active_import_l1": power_schema,
    "momentary_active_export_l1": power_schema,
    "momentary_active_import_l2": power_schema,
    "momentary_active_export_l2": power_schema,
    "momentary_active_import_l3": power_schema,
    "momentary_active_export_l3": power_schema,
    "momentary_reactive_import_l1": reactive_power_schema,
    "momentary_reactive_export_l1": reactive_power_schema,
    "momentary_reactive_import_l2": reactive_power_schema,
    "momentary_reactive_export_l2": reactive_power_schema,
    "momentary_reactive_import_l3": reactive_power_schema,
    "momentary_reactive_export_l3": reactive_power_schema,
    "voltage_l1": voltage_schema,
    "voltage_l2": voltage_schema,
    "voltage_l3": voltage_schema,
    "current_l1": current_schema,
    "current_l2": current_schema,
    "current_l3": current_schema,
}

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_P1READER_ID): cv.use_id(P1Reader),
        **{
            cv.Optional(name): factory()
            for name, factory in SENSOR_TYPES.items()
        },
    }
).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    hub = await cg.get_variable(config[CONF_P1READER_ID])

    for key, conf in config.items():
        if not isinstance(conf, dict):
            continue
        id = conf[CONF_ID]
        if id and id.type == sensor.Sensor:
            sens = await sensor.new_sensor(conf)
            cg.add(getattr(hub, f"set_sensor_{key}")(sens))
