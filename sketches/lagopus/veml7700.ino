#include <Adafruit_VEML7700.h>


Adafruit_VEML7700 veml;
bool veml_ok = false;
float veml_lux, veml_white;
uint16_t veml_als;

void veml7700_init()
{
    veml_ok = veml.begin();
    if (veml_ok) {
        PRINTLN("7: VEML7700  OK");
    } else {
        PRINTLN("7: VEML7700  ERROR");
    }
}

void veml7700_measure(char *buffer)
{
    if (! veml_ok) {
        sendResponse("0000");
        return;
    }

    sendResponse("0013"); // 3 values in 1 second
    veml_lux = veml.readLux(VEML_LUX_AUTO);
    veml_white = veml.readWhite();
    veml_als = veml.readALS();
    bool ok = !isnan(veml_lux) && !isnan(veml_white);
    if (ok)
        veml7700_data(buffer);
    else
        buffer[0] = '\0';
}

void veml7700_data(char *buffer)
{
    sprintf(buffer, "%+.2f%+.2f+%u", veml_lux, veml_white, veml_als);
}
