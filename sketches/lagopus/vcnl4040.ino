#include <Adafruit_VCNL4040.h>


Adafruit_VCNL4040 vcnl;
bool vcnl_ok = false;
uint16_t vcnl_prox, vcnl_lux, vcnl_white;

const VCNL4040_AmbientIntegration integration_times[] = {
    VCNL4040_AMBIENT_INTEGRATION_TIME_640MS,
    VCNL4040_AMBIENT_INTEGRATION_TIME_320MS,
    VCNL4040_AMBIENT_INTEGRATION_TIME_160MS,
    VCNL4040_AMBIENT_INTEGRATION_TIME_80MS,
};

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
    sendResponse("0063"); // 3 values in 6 seconds

#if defined(TEST) || defined(DEBUG)
    unsigned long t0 = millis();
#endif
    // To avoid saturation try different integration times until a valid value
    // is read
    for (int i=0; i < 4; i++) {
        vcnl.setAmbientIntegrationTime(integration_times[i]);
        vcnl_lux = vcnl.getLux();
        if (vcnl_lux < 65535) {
            break;
        }
    }

    vcnl_prox = vcnl.getProximity();
    vcnl_white = vcnl.getWhiteLight();

    bool ok = true;
    if (ok)
        vcnl4040_data(buffer);
    else
        buffer[0] = '\0';
#if defined(TEST) || defined(DEBUG)
    unsigned long t1 = millis(); Serial.print("time = "); Serial.print(t1-t0); Serial.println();
#endif
}

void vcnl4040_data(char *buffer)
{
    sprintf(buffer, "+%u+%u+%u", vcnl_prox, vcnl_lux, vcnl_white);
}
