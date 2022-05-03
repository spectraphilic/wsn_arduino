#include <Adafruit_AS7341.h>


Adafruit_AS7341 as7341;
bool as7341_ok = false;

void as7341_init()
{
    as7341_ok = as7341.begin();
    if (as7341_ok) {
        PRINTLN("0: AS7341    OK");
    } else {
        PRINTLN("0: AS7341    ERROR");
    }
}

void as7341_measure(char *buffer)
{
    if (! as7341_ok) {
        sendResponse("0000");
        return;
    }
    sendResponse("00110"); // 10 values in 1 second

#if defined(TEST) || defined(DEBUG)
    unsigned long t0 = millis();
#endif
    bool ok = as7341.readAllChannels();
    if (ok)
        as7341_data(buffer);
    else
        buffer[0] = '\0';
#if defined(TEST) || defined(DEBUG)
    unsigned long t1 = millis(); Serial.print("time = "); Serial.print(t1-t0); Serial.println();
#endif
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
