# esphome-p1reader
ESPHome custom component for reading P1 data from electricity meters. Designed for Swedish meters that implements the specification defined in the [Swedish Energy Industry Recommendation For Customer Interfaces](https://www.energiforetagen.se/forlag/elnat/branschrekommendation-for-lokalt-kundgranssnitt-for-elmatare/) version 1.3 and above.

Please note that the project currently doesn't support the Aidon meter from Tekniska Verken since that meter outputs the data in a binary format according to an earlier version (1.2) of the above mentioned recommendation.

## Verified meter hardware / supplier
* [Sagemcom T211](https://www.ellevio.se/globalassets/uploads/2020/nya-elmatare/ellevio_produktblad_fas3_t211_web2.pdf) / Ellevio

## Hardware
I have used an ESP-12 based NodeMCU for my circuit. Most ESP-12/ESP-32 based controllers would probably work. The P1 port on the meter provides 5V up to 250mA which makes it possible to power the circuit directly from the P1 port.

### Parts
- 1 NodeMCU or equivalent ESP-12 / ESP-32 microcontroller
- 1 BC547 NPN transistor
- 1 4.7kOhm Resistor
- 1 10kOhm Resistor
- 1 RJ12 6P6C port
- 1 RJ12 to RJ12 cable (6 wires)

### Wiring
The circuit is very simple, basically the 5V TX output on the P1 connector is converted to 3.3V and inverted by the transistor and connected to the UART0 RX pin on the microcontroller. The RTS (request to send) pin is pulled high so that data is sent continously and GND and 5V is taken from the P1 connector to drive the microcontroller.

![Wiring Diagram](images/wiring.png)

## Installation
Clone the repository and update the [p1reader.yaml](p1reader.yaml) with your own settings (wifi SSID and password and API password).

Prepare the microcontroller with ESPHome before you connect it to the circuit:
- Install the `esphome` [command line tool](https://esphome.io/guides/getting_started_command_line.html)
- Plug in the microcontroller to your USB port and run `esphome p1reader.yaml run` to flash the firmware
- Remove the USB connection and connect the microcontroller to the rest of the circuit and plug it into the P1 port.
- If everything works, your Home Assistant will now auto detect your new ESPHome integration.

You can check the logs by issuing `esphome p1reader.yaml logs` (or use the super awesome ESPHome dashboard available as a Hass.io add-on or standalone). The logs should output data similar to this every 10 seconds:
```
[23:03:23][D][data:291]: [1.8.0]: 00001587.242 kWh
[23:03:23][D][data:291]: [2.8.0]: 00000000.000 kWh
[23:03:23][D][data:291]: [3.8.0]: 00000005.420 kvarh
[23:03:23][D][data:291]: [4.8.0]: 00000269.077 kvarh
[23:03:23][D][data:291]: [1.7.0]: 0000.702 kW
[23:03:23][D][data:291]: [2.7.0]: 0000.000 kW
[23:03:23][D][data:291]: [3.7.0]: 0000.041 kvar
[23:03:23][D][data:291]: [4.7.0]: 0000.340 kvar
[23:03:23][D][data:291]: [21.7.0]: 0000.351 kW
[23:03:23][D][data:291]: [41.7.0]: 0000.187 kW
[23:03:23][D][data:291]: [61.7.0]: 0000.163 kW
[23:03:23][D][data:291]: [22.7.0]: 0000.000 kW
[23:03:23][D][data:291]: [42.7.0]: 0000.000 kW
[23:03:23][D][data:291]: [62.7.0]: 0000.000 kW
[23:03:23][D][data:291]: [23.7.0]: 0000.000 kvar
[23:03:23][D][data:291]: [43.7.0]: 0000.041 kvar
[23:03:23][D][data:291]: [63.7.0]: 0000.000 kvar
[23:03:23][D][data:291]: [24.7.0]: 0000.259 kvar
[23:03:23][D][data:291]: [44.7.0]: 0000.000 kvar
[23:03:23][D][data:291]: [64.7.0]: 0000.080 kvar
[23:03:23][D][data:291]: [32.7.0]: 233.8 V
[23:03:23][D][data:291]: [52.7.0]: 235.0 V
[23:03:23][D][data:291]: [72.7.0]: 234.9 V
[23:03:23][D][data:291]: [31.7.0]: 001.9 A
[23:03:23][D][data:291]: [51.7.0]: 000.8 A
[23:03:23][D][data:291]: [71.7.0]: 000.8 A
[23:03:23][D][crc:273]: CRC: 27DE = 27DE. PASS = YES
```

The last row contains the CRC check. If you constantly get invalid CRC there might be something wrong with the serial communication.

## Technical documentation
Specification overview:
https://www.tekniskaverken.se/siteassets/tekniska-verken/elnat/aidonfd-rj12-han-interface-se-v13a.cleaned.pdf

OBIS codes:
https://tech.enectiva.cz/en/installation-instructions/others/obis-codes-meaning/

P1 hardware info (in Dutch):
http://domoticx.com/p1-poort-slimme-meter-hardware/
