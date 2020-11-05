# esphome-p1reader
ESPHome custom component for reading P1 data from electricity meters. Designed for Swedish meters but will probably work for any compliant P1 port.

## Wiring
This code expects the data request pin (pin #2) on the RJ12 connection to always be pulled high +5V so that we get a continous flow of data every 10 seconds.
More info coming soon..

## Technical documentation
Specification overview:
https://www.tekniskaverken.se/siteassets/tekniska-verken/elnat/aidonfd-rj12-han-interface-se-v13a.cleaned.pdf

OBIS codes:
https://tech.enectiva.cz/en/installation-instructions/others/obis-codes-meaning/

P1 hardware info (in Dutch):
http://domoticx.com/p1-poort-slimme-meter-hardware/
