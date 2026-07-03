import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from esphome.const import (
    CONF_UART_ID, CONF_ID
)

CODEOWNERS = ["cadwal"]

MULTI_CONF = True

DEPENDENCIES = ["uart"]
AUTO_LOAD = ["sensor"]

CONF_P1READER_ID = "p1reader_id"
CONF_BUFFER_SIZE = "buffer_size"
CONF_PROTOCOL = "protocol"
CONF_REPEAT_TO_TX = "repeat_to_tx"

p1reader_ns = cg.esphome_ns.namespace("esphome::p1_reader")
P1Reader = p1reader_ns.class_("P1Reader", cg.PollingComponent, uart.UARTDevice)

CONFIG_SCHEMA = cv.All(
    cv.Schema(
        {
            cv.GenerateID(): cv.declare_id(P1Reader),
            cv.Optional(CONF_BUFFER_SIZE, default=60): cv.positive_not_null_int,
            cv.Optional(CONF_PROTOCOL, default="ascii"): cv.one_of("ascii", "hdlc", lower=True),
            cv.Optional(CONF_REPEAT_TO_TX, default=False): cv.boolean,
        }
    ).extend(cv.COMPONENT_SCHEMA).extend(uart.UART_DEVICE_SCHEMA),
    cv.only_with_arduino,
)


async def to_code(config):
    uart_component = await cg.get_variable(config[CONF_UART_ID])
    var = cg.new_Pvariable(config[CONF_ID], uart_component)
    await cg.register_component(var, config)

    cg.add(var.set_protocol_type(config[CONF_PROTOCOL]))
    cg.add(var.set_repeat_to_tx(config[CONF_REPEAT_TO_TX]))
    if config[CONF_PROTOCOL] == "ascii":
        cg.add_define("BUF_SIZE", config[CONF_BUFFER_SIZE])
    else:
        cg.add_define("BUF_SIZE", 4096)
