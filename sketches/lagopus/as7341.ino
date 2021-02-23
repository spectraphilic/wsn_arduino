#include <Adafruit_AS7341.h>


Adafruit_AS7341 as7341;
bool as7341_ok;

void as7341_init()
{
    as7341_ok = as7341.begin();
    if (as7341_ok) {
        PRINTLN("AS7341    OK");
    } else {
        PRINTLN("AS7341    ERROR");
    }
}

void as7341_measure()
{
    if (as7341_ok) {
        sendResponse("00110"); // 10 values in 1 second
        bool ok = as7341.readAllChannels();
        sensor = ok ? AS7341 : ABORT;
    } else {
        sendResponse("0000");
    }
}

void as7341_data(char *buffer)
{
    uint16_t as7341_channels[12];
    as7341.getAllChannels(as7341_channels);

    char *p = buffer;
    for (int i = 0; i < 10; i++) {
        p += sprintf(p, "+%u", as7341_channels[i]);
    }
}
