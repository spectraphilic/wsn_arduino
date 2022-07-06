#include <SparkFun_VL53L1X.h>


SFEVL53L1X vl;
bool vl_ok = false;
const unsigned int vl_distances_n = 15;
uint16_t vl_distances[vl_distances_n];

void vl53l1_init()
{
    vl_ok = ! vl.begin();
    if (vl_ok) {
        PRINTLN("9: VL54L1X   OK");
    } else {
        PRINTLN("9: VL54L1X   ERROR");
    }
}


void vl53l1_measure(char *buffer)
{
    if (! vl_ok) {
        sendResponse("0000");
        return;
    }
    sendResponse("00215"); // 15 values in 2 seconds

#if defined(TEST) || defined(DEBUG)
    unsigned long t0 = millis();
#endif
    vl.startRanging();

    // Wait for data ready
    for (int i = 0; i < vl_distances_n; i++) {
        while (! vl.checkForDataReady()) // XXX Implement a timeout??
            delay(1);

        vl_distances[i] = vl.getDistance();
    }

    vl.clearInterrupt();
    vl.stopRanging();

    vl53l1_data(buffer);
#if defined(TEST) || defined(DEBUG)
    unsigned long t1 = millis(); Serial.print("time = "); Serial.print(t1-t0); Serial.println();
#endif
}

void vl53l1_data(char *buffer)
{
    char *p = buffer;
    for (int i = 0; i < vl_distances_n; i++) {
        p += sprintf(p, "+%u", vl_distances[i]);
    }
}
