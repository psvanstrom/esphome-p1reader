# esphome-p1reader
ESPHome custom component for reading P1 data from electricity meters. Designed for Swedish meters that implement the specification defined in the [Swedish Energy Industry Recommendation For Customer Interfaces](https://www.energiforetagen.se/forlag/elnat/branschrekommendation-for-lokalt-kundgranssnitt-for-elmatare/) version 1.3 and above.

## ESPHome version
The current version in main is tested with ESPHome version `2024.12.4` and `2025.2.0`. Make sure your ESPHome version is up to date if you experience compile problems.
(NOTE: This component is now an External Component as the Custom Components API have been removed from ESPHome)

## Verified meter hardware / supplier
* [Sagemcom T211](https://www.ellevio.se/globalassets/content/el/elmatare-produktblad-b2c/ellevio_produktblad_fas3_t211_web2_.pdf) / Ellevio & Skånska Energi ([Info, port activation, etc.](https://www.ellevio.se/privat/om-din-el/elen-i-hemmet/forsta-din-elmatare/))
* [Landis+Gyr E360](https://eu.landisgyr.com/blog-se/e360-en-smart-matare-som-optimerarden-totala-agandekostnaden)
* [Itron A300](https://boraselnat.se/elnat/elmatarbyte-2020-2021/sa-har-fungerar-din-nya-elmatare/) / Borås Elnät
* [S34U18 (Sanxing SX631)](https://www.vattenfalleldistribution.se/globalassets/vattenfalleldistribution/kund-i-elnatet/matarbyte/nya-elmataren/vattenfall-eldistribution_anvandarmanual-elmatare.pdf) / Vattenfall
* [KAIFA MA304H4E](https://reko.nackaenergi.se/elmatarbyte/) / Nacka Energi
* [KAIFA CL109](https://www.oresundskraft.se/dags-for-matarbyte/) / Öresundskraft
* [Aidon](https://www.tekniskaverken.se/kundservice/dinamatare/snart-far-du-nya-matare/) / Tekniska Verken
* [Kamstrup Omnia](https://www.goteborgenergi.se/kundservice/elmatarbyte/sa-fungerar-din-elmatare) / Göteborgs Energi

*Note:* There's a bug in older E360 firmware, causing it to stop sending out data after a while. Check this comment for more info: https://github.com/psvanstrom/esphome-p1reader/issues/4#issuecomment-810794020
  
*Warning:*  Do not confuse KAIFA MA304H4**E** with MA304H4**D** as the latter uses M-Bus instead of P1. Apart from being incompatible protocols, M-Bus pin 1 exerts 27V instead of 5V and will fry your P1 equipment.

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
The circuit is very simple, basically the 5V TX output on the P1 connector is converted to 3.3V and inverted by the transistor and connected to the UART0 RX pin on the microcontroller. The RTS (request to send) pin is pulled high so that data is sent continuously and GND and 5V are taken from the P1 connector to drive the microcontroller.

#### Wiring NodeMCU ESP-12
![Wiring Diagram](images/wiring.png)

#### Wiring Wemos D1 mini
![image](https://user-images.githubusercontent.com/5547521/132756141-53941ed7-64f6-4c83-b0b0-6fc7c9634752.png)

#### Wiring barebone ESP-12 with added voltage regulators and capacitors.
The schematics show a 2SC1815 NPN transistor being used (because that's what I had laying around), will work just fine with either one of the transistors listed under the parts section. 
![Wiring Diagram](images/p1reader-barebone-ESP-12F.png)

### PCB and enclosures
#### Naesstrom
[Naesstrom](https://github.com/Naesstrom) has made a nice PCB layout for the P1 reader using a Wemos D1 mini as the controller along with a 3D printable enclosure. 

Check out the PCB here: https://oshwlab.com/Naesstrom/esphome-p1reader and the enclosure here: https://www.thingiverse.com/thing:4961372.

<p float="left">
    <img src="https://user-images.githubusercontent.com/5547521/128576100-648cd2b7-d728-4d8b-90be-46f7498d8136.png" height="300" />
    <img src="https://user-images.githubusercontent.com/5547521/132759466-f92bf190-ebaa-401d-bb54-330df5ba3ae0.png" height="300" /> 
</p>

#### EHjortberg
[EHjortberg](https://github.com/ehjortberg) has made an equally nice PCB layout based on an ESP07 module along with a 3D printable enclosure. Check it out here: https://github.com/ehjortberg/kicad-p1-port-thingie.

<p float="left">
  <img src="https://github.com/ehjortberg/kicad-p1-port-thingie/raw/master/images/p1-port-thingie-photo.jpg" width="400">
</p>

## Alternate hardware

Since most of the actual specification is, for all practical purposes, identical across Europe/EU (with the exception of Norway) we can use many kinds of different hardware that supports ESPHome and can interface with the P1 port. Both other custom made solutions and some commercial options. The key to using them is to combine them with the code here that handles the Swedish selection of data values sent from the smart meter. The DSMR module in ESPHome follows the Dutch specification for values to be used.

(Finland and Denmark seems to have the exact same configuration as Sweden)

### Running on ESP32

The ESP32 requires an external power supply since it cannot be powered by the low amperage available from the P1-port itself. Using the hardware UART on GPIO3 is required.

Use a pull-up resistor (1k) from RXD to 3V3.

```
uart:
    id: uart_bus
    rx_pin:
      number: GPIO3
      inverted: true
    baud_rate: 115200
```

![image](https://user-images.githubusercontent.com/36197/199937760-c6dce355-1e69-4b78-ae04-e2f6c9b2241e.png)

Image credit: https://github.com/Josverl/micropython-p1meter

### Running on SmartyReader P1

Weigu has designed [SmartyReader P1](http://weigu.lu/microcontroller/smartyReader_P1/index.html) that also can be running with this code and configuration with a few small adaptions.

This hardware runs on ESP8266 Wemos D1 mini pro but with less components.

#### Basic steps to run this code on SmartyReader P1:

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

### Running on Slimmelezer(+)

Marcel Zuidwijk has designed [Slimmelezer+](https://www.zuidwijk.com/product/slimmelezer-plus/) that also can be used with this code and combined with the standard configuration for the Slimmelezer+. (Also the non + version works fine, the software is compatible).

It is designed as a custom board with a P1 interface that is software equivalent with the Wemos D1 Mini.

#### Basic steps to run this code on the Slimmelezer+:

1. Set the board to Wemos D1 mini
```
board: d1_mini
```

2. Adjust the UART section to set the rx_pin used by the Slimmelezer
```
uart:
    id: uart_bus
    baud_rate: 115200
    rx_pin: D7
    rx_buffer_size: 3072    
```

[Sample configuration](./samples/slimmelezer.yaml), uses !secret for all site specific configuration, see below.

### Running on Currently One (v1.1)

Stopgap AB is selling [Currently One](https://currently.one) which can be used with this code and a modified configuration.

It is designed using a ESP32-C3 with the P1 data pin on GPIO6.

This code is ideal for customers who only wants to get their data into Home Assistant, without any cloud service or custom mobile app whatsoever. 

The modified config can be found at [this fork](https://github.com/andreanielsen83/esphome-p1reader).

## Installation

Clone the repository and create a companion `secrets.yaml` file with the following fields:

Create a companion `secrets.yaml` file with the following fields:
```
wifi_ssid: <your wifi SSID>
wifi_password: <your wifi password>
fallback_password: <fallback AP password>
encryption_key: the encryption key shared with HA
ota_password: <The OTA password>
```
Check [the Native API Component chapter of the ESPHome documentation](https://esphome.io/components/api.html#configuration-variables) for more info on the encryption key, this page also let you easily generate an encryption key.

Make sure to place the `secrets.yaml` file in the root path of the cloned project. The `fallback_password` and `ota_password` fields can be set to any password before doing the initial upload of the firmware.

If your electricity supplier is using an Aidon 6442SE or Aidon 653X meter, they might still be using the HDLC protocol rather than the ASCII format for the meter data on the P1 port. Use the [Sample configuration for HDLC](./samples/p1reader_hdlc.yaml) to handle this setup. It configures a different input parser to handle the HDLC protocol.

Prepare the microcontroller with ESPHome before you connect it to the circuit:
- Install the `esphome` [command line tool](https://esphome.io/guides/getting_started_command_line.html)
- Plug in the microcontroller to your USB port and run `esphome run p1reader.yaml` to flash the firmware
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

Note that the default is the `INFO` loglevel since logging affects performance.

The last row contains the CRC check. If you constantly get invalid CRC there might be something wrong with the serial communication.

## Technical documentation
Specification overview:
https://www.tekniskaverken.se/siteassets/tekniska-verken/elnat/elmatare-och-elanvandning/aidon-rj12-han-interface-v17a.pdf

- OBIS codes:
https://tech.enectiva.cz/en/installation-instructions/others/obis-codes-meaning/

- P1 hardware info (in Dutch):
http://domoticx.com/p1-poort-slimme-meter-hardware/

- Original Dutch specification (P1 Companion Standard - DSMR 5.0.2).
https://www.netbeheernederland.nl/_upload/Files/Slimme_meter_15_a727fce1f1.pdf

- Luxembourg specification (E-Meter P1 Specification 1.1.2)
https://www.luxmetering.lu/pdf/SPEC%20-%20E-Meter_P1_specification_20210308.pdf

- Belgian specification
https://www.fluvius.be/sites/fluvius/files/2020-03/1901-fluvius-technical-specification-user-ports-digital-meter.pdf
https://www.fluvius.be/sites/fluvius/files/2019-12/e-mucs_h_ed_1_3.pdf

- Swedish specification (Branschrekommendation för lokalt kundgränssnitt för elmätare 2.0)
https://www.energiforetagen.se/globalassets/energiforetagen/det-erbjuder-vi/kurser-och-konferenser/elnat/branschrekommendation-lokalt-granssnitt-v2_0-201912.pdf

- In Lithuania Dutch specification is used (in Lithuanian):
  https://ismaniejiskaitikliai.lt/dazniausiai-uzduodami-klausimai/1#c-8/t-74
 



