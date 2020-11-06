# esphome-p1reader
ESPHome custom component for reading P1 data from electricity meters. Designed for Swedish meters but will probably work for any compliant P1 port.

## Hardware
I have used an ESP-12 based NodeMCU for my circuit. Most ESP-12/ESP-32 based controllers would probably work. The P1 port on the meter provides 5V up to 250mA which makes it possible to power the circuit directly from the P1 port.

To convert the 5V signal to 3.3V suitable for the RX pin on the microcontroller I've used a bidirectional logic level converter. This could however be replaced with a simple voltage divider circuit made out of two resistors as described here: https://learn.sparkfun.com/tutorials/voltage-dividers/all even though this might introduce problems due to the high bitrate (115200 bps).

### Parts
- 1 NodeMCU or equivalent ESP-12 / ESP-32 microcontroller
- 1 BC547 NPN transistor
- 1 4.7kOhm Resistor
- 1 10kOhm Resistor
- 1 Bidirectional logic level converter 5V <-> 3.3V (example: https://www.sparkfun.com/products/12009)
- 1 RJ12 6P6C port
- 1 RJ12 to RJ12 cable (6 wires)

### Wiring
The RTS (request to send) pin should be connected directly to 5V so that data is sent continously. The signals from the P1 port needs to be inverted, which is handled by the BC547 transistor.



## Technical documentation
Specification overview:
https://www.tekniskaverken.se/siteassets/tekniska-verken/elnat/aidonfd-rj12-han-interface-se-v13a.cleaned.pdf

OBIS codes:
https://tech.enectiva.cz/en/installation-instructions/others/obis-codes-meaning/

P1 hardware info (in Dutch):
http://domoticx.com/p1-poort-slimme-meter-hardware/
