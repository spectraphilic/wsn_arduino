#include <SparkFun_VL53L1X.h>


SFEVL53L1X vl;
bool vl_ok;
const unsigned int vl_distances_n = 15;
int vl_distances[vl_distances_n];

void vl53l1_init()
{
    vl_ok = ! vl.begin();
    if (vl_ok) {
        PRINTLN("9: VL54L1X   OK");
    } else {
        PRINTLN("9: VL54L1X   ERROR");
    }
}


void vl53l1_measure()
{
    if (! vl_ok) {
        sendResponse("0000");
        return;
    }

    sendResponse("00215"); // 15 values in 2 seconds
    vl.startRanging();

    // Wait for data ready
    for (int i = 0; i < vl_distances_n; i++) {
        while (! vl.checkForDataReady()) // XXX Implement a timeout??
            delay(1);

        vl_distances[i] = vl.getDistance();
    }

    vl.clearInterrupt();
    vl.stopRanging();
    sensor = VL53L1;
}

void vl53l1_data(char *buffer)
{
    char *p = buffer;
    for (int i = 0; i < vl_distances_n; i++) {
        p += sprintf(p, "%+d", vl_distances[i]);
    }
}
