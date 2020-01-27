# digoo-1w

This is firmware for a Digispark (or clone) ATTiny85 based board to decode 433MHz transmissions from a [Digoo DG-R8H](https://www.banggood.com/Digoo-DG-R8H-433MHz-Wireless-Digital-Hygrometer-Thermometer-Weather-Station-Sensor-for-TH11300-8380-p-1178108.html) wireless temperature/humidity weather station monitor and output them over a transmit only serial connection. Additionally it will do a periodic (once a minute) poll of an attached DS18B20 1-Wire temperature sensor and output its details. The intended use is to provide reception of remote temperature sensors (the DG-R8Hs) as well as a local temperature sensor (the DS18B20).

Although the Digispark can drive USB with the [V-USB](https://www.obdev.at/products/vusb/) software implementation the timing constraints make it difficult to reliably detect and parse the 433MHz transmissions. Using a software driven TX only UART pin allows priority to be given to the decoding on received signals. Connection to a computer can then be via a cheap TTL USB serial dongle - testing has been performed using a CH340G based device which cost 50p.

Both the 433MHz receiver and the 1-wire sensor are optional, allowing the same firmware to be used on a device that only has one of these connected without modification.

## Pins

The following connections are used on the Digispark in addition to GND/5V:

* PB0 - 1-Wire connection
* PB1 - TX serial out (230400, 8N1)
* PB2 - 433MHz receiver in

## Author

* [Jonathan McDowell](https://www.earth.li/~noodles/blog/)

## License

This project is licensed under the GPL 3+ license, see [COPYING](COPYING) for details.

## Download

Code is available on [GitHub](https://github.com/u1f35c/digoo-1w).
