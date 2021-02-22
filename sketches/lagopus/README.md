# SDI-12

Install the following libraries with the Arduni library manager:
- SDI-12
- Adafruit AS7341
- Adafruit BME280
- Adafruit ICM20X
- Adafruit MLX90614
- Adafruit SHT31
- Adafruit TMP117
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

To do:

- Implement and use concurrent measurements instead of regular measurements.
  Because they allow to return more than 9 data points, and the response size
  limit is 75 chars instead of 35 chars.

- Send at most 35 chars (or 75 chars for concurrent measurements) in response
  to a measure/data sequence. If there are more data points that fit in 35/75
  chars the data logger must issue aD1...aD9 commands to retrieve all the data.

Standard required features not yet implemented:

- Service requests
- Concurrent measurements (aC!atttnn\r\n)
- Continuous Measurements (aR0!)
- Verification (aV!)
- CRC
- Extended commands
- High volume commands

For the AS7341 and ICM20X we send more data than allowed by the SDI-12 spec:

- More than 9 data points
