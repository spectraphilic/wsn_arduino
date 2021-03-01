#include <Adafruit_VCNL4040.h>


Adafruit_VCNL4040 vcnl;
bool vcnl_ok = false;
uint16_t vcnl_prox, vcnl_lux, vcnl_white;


void vcnl4040_init()
{
    vcnl_ok = vcnl.begin();
    if (vcnl_ok) {
        PRINTLN("6: VCNL4040  OK");
    } else {
        PRINTLN("6: VCNL4040  ERROR");
    }
}

void vcnl4040_measure(char *buffer)
{
    if (! vcnl_ok) {
        sendResponse("0000");
        return;
    }

    sendResponse("0013"); // 3 values in 1 second
    vcnl_prox = vcnl.getProximity();
    vcnl_lux = vcnl.getLux();
    vcnl_white = vcnl.getWhiteLight();
    bool ok = true;
    if (ok)
        vcnl4040_data(buffer);
    else
        buffer[0] = '\0';
}

void vcnl4040_data(char *buffer)
{
    sprintf(buffer, "+%u+%u+%u", vcnl_prox, vcnl_lux, vcnl_white);
}
