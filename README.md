# esphome-p1reader
ESPHome custom component for reading P1 data from electricity meters. Designed for Swedish meters that implements the specification defined in the [Swedish Energy Industry Recommendation For Customer Interfaces](https://www.energiforetagen.se/forlag/elnat/branschrekommendation-for-lokalt-kundgranssnitt-for-elmatare/) version 1.3 and above.

Please note that the project currently doesn't support the Aidon meter from Tekniska Verken since that meter outputs the data in a binary format according to an earlier version (1.2) of the above mentioned recommendation.

## ESPHome version
The current version in main is tested with ESPHome version `2022.2.6`. Make sure your ESPHome version is up to date if you experience compile problems.

## Verified meter hardware / supplier
* [Sagemcom T211](https://www.ellevio.se/globalassets/uploads/2020/nya-elmatare/ellevio_produktblad_fas3_t211_web2.pdf) / Ellevio
* [Landis+Gyr E360](https://eu.landisgyr.com/blog-se/e360-en-smart-matare-som-optimerarden-totala-agandekostnaden)
* [Itron A300](https://boraselnat.se/elnat/elmatarbyte-2020-2021/sa-har-fungerar-din-nya-elmatare/) / Borås Elnät
* [S34U18 (Sanxing SX631)](https://www.vattenfalleldistribution.se/matarbyte/nya-elmataren/) / Vattenfall 
* [KAIFA MA304H4E](https://reko.nackaenergi.se/elmatarbyte/) / Nacka Energi

*Note:* There's currently a bug in the E360 firmware, causing it to stop sending out data after a while. Check this comment for more info: https://github.com/psvanstrom/esphome-p1reader/issues/4#issuecomment-810794020

## Hardware
I have used an ESP-12 based NodeMCU for my circuit, another alternative is the cheaper Wemos D1 mini but most ESP-based controllers would probably work. The P1 port on the meter provides 5V up to 250mA which makes it possible to power the circuit directly from the P1 port.

### Parts
- 1 NodeMCU, Wemos D1 mini or equivalent ESP-12 / ESP-32 microcontroller
- 1 BC547 / 2N3904 NPN transistor
- 1 4.7kOhm Resistor
- 1 10kOhm Resistor
- 1 RJ12 6P6C port
- 1 RJ12 to RJ12 cable (6 wires)

### Wiring
The circuit is very simple, basically the 5V TX output on the P1 connector is converted to 3.3V and inverted by the transistor and connected to the UART0 RX pin on the microcontroller. The RTS (request to send) pin is pulled high so that data is sent continously and GND and 5V is taken from the P1 connector to drive the microcontroller.

#### Wiring NodeMCU ESP-12
![Wiring Diagram](images/wiring.png)

#### Wiring Wemos D1 mini
![image](https://user-images.githubusercontent.com/5547521/132756141-53941ed7-64f6-4c83-b0b0-6fc7c9634752.png)

### PCB
[Naesstrom](https://github.com/Naesstrom) has made a nice PCB layout for the P1 reader using a Wemos D1 mini as the controller. Check it out here: https://oshwlab.com/Naesstrom/esphome-p1reader.

<img src="https://user-images.githubusercontent.com/5547521/128576100-648cd2b7-d728-4d8b-90be-46f7498d8136.png" width="200" height="374">

### Enclosure
[Naesstrom](https://github.com/Naesstrom) has also created a very nice enclosure to be 3D-printed which can be found here: https://www.thingiverse.com/thing:4961372.

![image](https://user-images.githubusercontent.com/5547521/132759466-f92bf190-ebaa-401d-bb54-330df5ba3ae0.png)

## Optional hardware
Weigu has designed [SmartyReader P1](http://weigu.lu/microcontroller/smartyReader_P1/index.html) that also can be running with this code and configuration with a few small adaptions.

This hardware runs on ESP8266 Wemos D1 mini pro but with less components.

### Basic steps to run this code on SmartyReader P1:

1. Set the board to Wemos D1 mini pro
```
board: d1_mini_pro
```

2. Adjust the UART section to invert the RX pin (removed TX pin config since it is not used).
```
uart:
    id: uart_bus
    rx_pin:
      number: 3
      inverted: true
    baud_rate: 115200
```

~~Note that the inverted flag is only supported in ESPHome beta as of now.~~
~~Monitor [this PR](https://github.com/esphome/esphome/pull/1727) to follow if it is released to general version.~~
_inverted flag feature has been added in ESPHome 2021.12.0 released on 11th December 2021._

## Installation
Clone the repository and create a companion `secrets.yaml` file with the following fields:
```
wifi_ssid: <your wifi SSID>
wifi_password: <your wifi password>
fallback_password: <fallback AP password>
hass_api_password: <the Home Assistant API password>
ota_password: <The OTA password>
```
Make sure to place the `secrets.yaml` file in the root path of the cloned project. The `fallback_password` and `ota_password` fields can be set to any password before doing the initial upload of the firmware.



Prepare the microcontroller with ESPHome before you connect it to the circuit:
- Install the `esphome` [command line tool](https://esphome.io/guides/getting_started_command_line.html)
- Plug in the microcontroller to your USB port and run `esphome p1reader.yaml run` to flash the firmware
- Remove the USB connection and connect the microcontroller to the rest of the circuit and plug it into the P1 port.
- If everything works, your Home Assistant will now auto detect your new ESPHome integration.

You can check the logs by issuing `esphome p1reader.yaml logs` (or use the super awesome ESPHome dashboard available as a Hass.io add-on or standalone). The logs should output data similar to this every 10 seconds when using `DEBUG` loglevel:
```
[18:40:01][D][data:264]: /ELL5\253833635_A
[18:40:01][D][data:264]:
[18:40:01][D][data:264]: 0-0:1.0.0(210217184019W)
[18:40:01][D][data:264]: 1-0:1.8.0(00006678.394*kWh)
[18:40:01][D][data:264]: 1-0:2.8.0(00000000.000*kWh)
[18:40:01][D][data:264]: 1-0:3.8.0(00000021.988*kvarh)
[18:40:01][D][data:264]: 1-0:4.8.0(00001020.971*kvarh)
[18:40:01][D][data:264]: 1-0:1.7.0(0001.727*kW)
[18:40:01][D][data:264]: 1-0:2.7.0(0000.000*kW)
[18:40:01][D][data:264]: 1-0:3.7.0(0000.000*kvar)
[18:40:01][D][data:264]: 1-0:4.7.0(0000.309*kvar)
[18:40:01][D][data:264]: 1-0:21.7.0(0001.023*kW)
[18:40:01][D][data:264]: 1-0:41.7.0(0000.350*kW)
[18:40:01][D][data:264]: 1-0:61.7.0(0000.353*kW)
[18:40:01][D][data:264]: 1-0:22.7.0(0000.000*kW)
[18:40:01][D][data:264]: 1-0:42.7.0(0000.000*kW)
[18:40:01][D][data:264]: 1-0:62.7.0(0000.000*kW)
[18:40:01][D][data:264]: 1-0:23.7.0(0000.000*kvar)
[18:40:01][D][data:264]: 1-0:43.7.0(0000.000*kvar)
[18:40:01][D][data:264]: 1-0:63.7.0(0000.000*kvar)
[18:40:01][D][data:264]: 1-0:24.7.0(0000.009*kvar)
[18:40:01][D][data:264]: 1-0:44.7.0(0000.161*kvar)
[18:40:01][D][data:264]: 1-0:64.7.0(0000.138*kvar)
[18:40:01][D][data:264]: 1-0:32.7.0(240.3*V)
[18:40:01][D][data:264]: 1-0:52.7.0(240.1*V)
[18:40:01][D][data:264]: 1-0:72.7.0(241.3*V)
[18:40:01][D][data:264]: 1-0:31.7.0(004.2*A)
[18:40:01][D][data:264]: 1-0:51.7.0(001.6*A)
[18:40:01][D][data:264]: 1-0:71.7.0(001.7*A)
[18:40:01][D][data:264]: !7945
[18:40:01][I][crc:275]: Telegram read. CRC: 7945 = 7945. PASS = YES
```

The last row contains the CRC check. If you constantly get invalid CRC there might be something wrong with the serial communication.

## Technical documentation
Specification overview:
https://www.tekniskaverken.se/siteassets/tekniska-verken/elnat/aidonfd-rj12-han-interface-se-v13a.cleaned.pdf

OBIS codes:
https://tech.enectiva.cz/en/installation-instructions/others/obis-codes-meaning/

P1 hardware info (in Dutch):
http://domoticx.com/p1-poort-slimme-meter-hardware/
