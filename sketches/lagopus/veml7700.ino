#include <Adafruit_VEML7700.h>


Adafruit_VEML7700 veml;
bool veml_ok;
float veml_lux, veml_white; uint16_t veml_als;

void veml7700_init()
{
    veml_ok = veml.begin();
    if (veml_ok) {
        PRINTLN("VEML7700  OK");
    } else {
        PRINTLN("VEML7700  ERROR");
    }
}

void veml7700_measure()
{
    if (veml_ok) {
        sendResponse("0013"); // 3 values in 1 second
        veml_lux = veml.readLux();
        veml_white = veml.readWhite();
        veml_als = veml.readALS();
        sensor = VEML7700;
    } else {
        sendResponse("0000");
    }
}

void veml7700_data(char *buffer)
{
    sprintf(buffer, "%+.2f%+.2f%+u", veml_lux, veml_white, veml_als);
}