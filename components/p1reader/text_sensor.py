import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import text_sensor
from esphome.const import CONF_INTERNAL
from . import P1Reader, CONF_P1READER_ID

AUTO_LOAD = ["p1reader"]

CONFIG_SCHEMA = cv.Schema(
    {
        cv.GenerateID(CONF_P1READER_ID): cv.use_id(P1Reader),
    }
).extend(cv.COMPONENT_SCHEMA)


async def to_code(config):
    hub = await cg.get_variable(config[CONF_P1READER_ID])

    for key, conf in config.items():
        if not isinstance(conf, dict):
            continue
        id = conf.get("id")
        if id and id.type == text_sensor.TextSensor:
            var = await text_sensor.new_text_sensor(conf)
            cg.add(getattr(hub, f"set_{key}")(var))
