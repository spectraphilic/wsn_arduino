#include <Adafruit_SHT31.h>


Adafruit_SHT31 sht31;
bool sht31_ok;
float sht_t, sht_h;

void sht31_init()
{
    sht31_ok = sht31.begin(0x44); // Set to 0x45 for alternate i2c addr
    if (sht31_ok) {
        PRINTLN("SHT31     OK");
        // Enable the heater for 1s
        sht31.heater(true);
        delay(1000);
        sht31.heater(false);
    } else {
        PRINTLN("SHT31     ERROR");
    }
}

void sht31_measure()
{
    if (sht31_ok) {
        sendResponse("0012"); // 2 values in 1 second
        sensor = SHT31;
        sht_t = sht31.readTemperature();
        sht_h = sht31.readHumidity();
    } else {
        sendResponse("0000");
    }
}

void sht31_data(char *buffer)
{
    if (isnan(sht_t) || isnan(sht_h)) {
        // TODO What should we send?
    }
    sprintf(buffer, "%+.2f%+.2f", sht_t, sht_h);
}
