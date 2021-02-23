# SDI-12

Install the following libraries with the Arduni library manager:
- SDI-12
- Adafruit AS7341
- Adafruit BME280
- Adafruit ICM20X
- Adafruit MLX90614
- Adafruit SHT31
- Adafruit TMP117
- Adafruit VCNL4040
- Adafruit VEML7700
- Sparkfun VL53L1X

Sensors:
- 0x44 SHT31
- 0x48 TMP117
- 0x77 BME280
- 0x29 VL53L1X
- 0x5A MLX90614


# Pins

Data:
- Waspomote     : DIGITAL8
- Feather-M0    : D5


# Links

SDI-12 library
https://github.com/EnviroDIY/Arduino-SDI-12

SDI-12 slave example
`https://github.com/EnviroDIY/Arduino-SDI-12/blob/master/examples/h_SDI-12_slave_implementation/h_SDI-12_slave_implementation.ino`

Feather-M0 Adalogger pins:
https://learn.adafruit.com/adafruit-feather-m0-adalogger/pinouts

QT Py pins:
https://learn.adafruit.com/adafruit-qt-py/pinouts

Waspmote pins (page 57):
`https://www.libelium.com/downloads/documentation/waspmote_technical_guide.pdf`


# TODO

Ways we deviate from the SDI-12 standard:

- We send more than 9 datapoints to M commands (AS7341 and ICM20X)
- We send more than 35 chars to M-D commands (AS7341 and ICM20X)
- We don't support some required features (see below)

Required features (from 1.3 version) we don't support:

- Service requests
- Concurrent measurements (aC!atttnn\r\n)
- Continuous Measurements (aR0!)
- Verification (aV!)
- CRC

Things we may do:

- Use concurrent commands, which allow for more data points (1-99) are longer D
  responses (75 chars)
- Support D1-9 commands, send no more than 35/75 chars for a single D command.
- Support service requests, for faster communication.
