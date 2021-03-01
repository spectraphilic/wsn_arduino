#include <Adafruit_ICM20948.h>


Adafruit_ICM20948 icm;
bool icm_ok = false;
sensors_event_t icm_accel, icm_gyro, icm_mag, icm_temp;


void icm_init()
{
    icm_ok = icm.begin_I2C();
    if (icm_ok) {
        PRINTLN("2: ICM20948  OK");
    } else {
        PRINTLN("2: ICM20948  ERROR");
    }
}

void icm_measure(char *buffer)
{
    if (! icm_ok) {
        sendResponse("0000");
        return;
    }

    sendResponse("00110"); // 10 values in 1 second
    bool ok = icm.getEvent(&icm_accel, &icm_gyro, &icm_temp, &icm_mag);
    if (ok)
        icm_data(buffer);
    else
        buffer[0] = '\0';
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
