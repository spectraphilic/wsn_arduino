#include <Adafruit_BME280.h>


Adafruit_BME280 bme;
bool bme_ok;
float bme_t, bme_p, bme_h;

void bme280_init()
{
    bme_ok = bme.begin(0x77);
    if (bme_ok) {
        PRINTLN("BME280    OK");
    } else {
        PRINTLN("BME280    ERROR");
    }
}

void bme280_measure()
{
    if (bme_ok) {
        sendResponse("0013"); // 3 values in 1 second
        sensor = BME280;
        bme_t = bme.readTemperature();
        bme_p = bme.readPressure() / 100.0F;
        bme_h = bme.readHumidity();
    } else {
        sendResponse("0000");
    }
}

void bme280_data(char *buffer)
{
    sprintf(buffer, "%+.2f%+.2f%+.2f", bme_t, bme_p, bme_h);
}
