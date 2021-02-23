#include <SparkFun_VL53L1X.h>


SFEVL53L1X vl;
bool vl_ok;
int vl_distance;

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
    if (vl_ok) {
        sendResponse("0011"); // 1 value in 1 second
        vl.startRanging();
        vl_distance = vl.getDistance();
        vl.stopRanging();
        sensor = VL53L1;
    } else {
        sendResponse("0000");
    }
}

void vl53l1_data(char *buffer)
{
    sprintf(buffer, "%+d", vl_distance);
}
