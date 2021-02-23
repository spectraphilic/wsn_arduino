#include <Adafruit_ICM20948.h>


Adafruit_ICM20948 icm;
bool icm_ok;
sensors_event_t icm_accel, icm_gyro, icm_mag, icm_temp;


void icm_init()
{
    icm_ok = icm.begin_I2C();
    if (icm_ok) {
        PRINTLN("ICM20948  OK");
    } else {
        PRINTLN("ICM20948  ERROR");
    }
}

void icm_measure()
{
    if (icm_ok) {
        sendResponse("00110"); // 10 values in 1 second
        bool ok = icm.getEvent(&icm_accel, &icm_gyro, &icm_temp, &icm_mag);
        sensor = ok ? ICM20X : ABORT;
    } else {
        sendResponse("0000");
    }
}

void icm_data(char *buffer)
{
    sprintf(buffer, "%+.2f%+.2f%+.2f%+.2f%+.2f%+.2f%+.2f%+.2f%+.2f%+.2f",
            icm_temp.temperature,
            icm_accel.acceleration.x,
            icm_accel.acceleration.y,
            icm_accel.acceleration.z,
            icm_mag.magnetic.x,
            icm_mag.magnetic.y,
            icm_mag.magnetic.z,
            icm_gyro.gyro.x,
            icm_gyro.gyro.y,
            icm_gyro.gyro.z);
}
