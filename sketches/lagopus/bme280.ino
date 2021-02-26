#include <Adafruit_BME280.h>


Adafruit_BME280 bme;
bool bme_ok = false;
float bme_t, bme_p, bme_h;

void bme280_init()
{
    bme_ok = bme.begin(0x77);
    if (bme_ok) {
        PRINTLN("1: BME280    OK");
    } else {
        PRINTLN("1: BME280    ERROR");
    }
}

void bme280_measure()
{
    if (bme_ok) {
        sendResponse("0013"); // 3 values in 1 second
        bme_t = bme.readTemperature();
        bme_h = bme.readHumidity();
        bme_p = bme.readPressure() / 100.0F;
        bool ok = !isnan(bme_t) && !isnan(bme_h) && !isnan(bme_p);
        sensor = ok ? BME280 : ABORT;
    } else {
        sendResponse("0000");
    }
}

void bme280_data(char *buffer)
{
    sprintf(buffer, "%+.2f%+.2f%+.2f", bme_t, bme_h, bme_p);
}
