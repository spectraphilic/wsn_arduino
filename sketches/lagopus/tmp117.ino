#include <Adafruit_TMP117.h>


Adafruit_TMP117 tmp117;
bool tmp117_ok = false;
sensors_event_t tmp117_event;

void tmp117_init()
{
    tmp117_ok = tmp117.begin();
    if (tmp117_ok) {
        PRINTLN("5: TMP117    OK");
    } else {
        PRINTLN("5: TMP117    ERROR");
    }
}

void tmp117_measure(char *buffer)
{
    if (! tmp117_ok) {
        sendResponse("0000");
        return;
    }

    sendResponse("0011"); // 1 value in 1 second
    bool ok = tmp117.getEvent(&tmp117_event);
    if (ok)
        tmp117_data(buffer);
    else
        buffer[0] = '\0';
}

void tmp117_data(char *buffer)
{
    sprintf(buffer, "%+.2f", tmp117_event.temperature);
}
