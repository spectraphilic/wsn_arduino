/**
 * Derived from:
 * https://github.com/EnviroDIY/Arduino-SDI-12/blob/master/examples/h_SDI-12_slave_implementation/h_SDI-12_slave_implementation.ino
 *
 * Feather-M0 Adalogger pins:
 * https://learn.adafruit.com/adafruit-feather-m0-adalogger/pinouts
 *
 * QT Py pins:
 * https://learn.adafruit.com/adafruit-qt-py/pinouts
 */

#include <SDI12.h>

#include <Adafruit_AS7341.h>
#include <Adafruit_BME280.h>
#include <Adafruit_ICM20948.h>
#include <Adafruit_MLX90614.h>
#include <Adafruit_SHT31.h>
#include <Adafruit_TMP117.h>
#include <Adafruit_VEML7700.h>
#include <SparkFun_VL53L1X.h>


//#define BLINK
//#define DEBUG
#define DATA_PIN 5   /*!< The pin of the SDI-12 data bus */
#define POWER_PIN -1 /*!< The sensor power pin (or -1 if not switching power) */

#ifdef DEBUG
    #define print(...) Serial.print(__VA_ARGS__)
    #define println(...) Serial.println(__VA_ARGS__)
#else
    #define print(...)
    #define println(...)
#endif

// Global objects
SDI12 sdi12(DATA_PIN);
Adafruit_AS7341 as7341;
Adafruit_BME280 bme;
Adafruit_ICM20948 icm;
Adafruit_MLX90614 mlx;
Adafruit_SHT31 sht31;
Adafruit_TMP117 tmp117;
Adafruit_VEML7700 veml;
SFEVL53L1X vl;

bool as7341_ok, bme_ok, icm_ok, mlx_ok, sht31_ok, tmp117_ok, veml_ok, vl_ok;


// States of the state machine for parsing SDI-12 messages
enum states {
    S_0, // initial state (empty string)
    S_Q, // ?
    S_a,
    S_aI,
    S_aA,
    S_aAb,
    S_aM,  // AS7341
    S_aM1, // BME280
    S_aM2, // ICM20X
    S_aM3, // MLX90614
    S_aM4, // SHT31
    S_aM5, // TMP117
    S_aM6, // VEML7700
    S_aM7, // VL53L1
    S_aMn, // A SDI-12 sensor must reply to all aMn commands
    S_aD,
    S_aD0,
};

enum sensors {
    AS7341,
    BME280,
    ICM20X,
    MLX90614,
    SHT31,
    TMP117,
    VEML7700,
    VL53L1,
};

// Global variables
char address = '5';
enum states state;
enum sensors sensor;


void as7341_init()
{
    as7341_ok = as7341.begin();
    if (as7341_ok) {
        println("AS7341    OK");
    } else {
        println("AS7341    ERROR");
    }
}

void bme280_init()
{
    bme_ok = bme.begin(0x77);
    if (bme_ok) {
        println("BME280    OK");
    } else {
        println("BME280    ERROR");
    }
}

void icm_init()
{
    icm_ok = icm.begin_I2C();
    if (icm_ok) {
        println("ICM20948  OK");
    } else {
        println("ICM20948  ERROR");
    }
}

void mlx_init()
{
    mlx_ok = mlx.begin();
    if (mlx_ok) {
        println("MLX90614  OK");
    } else {
        println("MLX90614  ERROR");
    }
}

void sht31_init()
{
    sht31_ok = sht31.begin(0x44); // Set to 0x45 for alternate i2c addr
    if (sht31_ok) {
        println("SHT31     OK");
        // Enable the heater for 1s
        sht31.heater(true);
        delay(1000);
        sht31.heater(false);
    } else {
        println("SHT31     ERROR");
    }
}

void tmp117_init()
{
    tmp117_ok = tmp117.begin();
    if (tmp117_ok) {
        println("TMP117    OK");
    } else {
        println("TMP117    ERROR");
    }
}

void veml7700_init()
{
    veml_ok = veml.begin();
    if (veml_ok) {
        println("VEML7700  OK");
    } else {
        println("VEML7700  ERROR");
    }
}

void vl53l1_init()
{
    vl_ok = ! vl.begin();
    if (vl_ok) {
        println("VL54L1X   OK");
    } else {
        println("VL54L1X   ERROR");
    }
}


void setup()
{
#ifdef BLINK
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
#endif

#ifdef DEBUG
    Serial.begin(9600); // The baudrate of Serial monitor is set in 9600
    while (!Serial); // Waiting for Serial Monitor
#endif

    // Initialize SDI-12 interface
    sdi12.begin();
    delay(500);
    sdi12.forceListen();  // sets SDIPIN as input to prepare for incoming message
    println("SDI-12    OK");

    // Intialize sensors
    as7341_init();
    bme280_init();
    icm_init();
    mlx_init();
    sht31_init();
    tmp117_init();
    veml7700_init();
    vl53l1_init();
}


int getChar()
{
    while (1) {
        int value = sdi12.read();
        if (value >= 0) {
            return value;
        }
    }
}


void sendResponse(const char *msg)
{
    char response[81];
    response[0] = address;
    strcpy(response+1, msg);
    strcat(response, "\r\n");
    sdi12.sendResponse(response);
    state = S_0; // Reset automata state
}


void loop()
{
    int c;
    char newAddress;
    char buffer[80];
    char *p;
    bool ok;

    uint16_t as7341_channels[12];
    float bme_t, bme_p, bme_h;
    double mlx_o, mlx_a;
    float sht_t, sht_h;
    float veml_lux, veml_white; uint16_t veml_als;
    sensors_event_t tmp117_event;
    sensors_event_t icm_accel, icm_gyro, icm_mag, icm_temp;
    int vl_distance;

    state = S_0;
    while (1) {
        c = getChar();
        switch (state) {
            case S_0:
                if (c == '?') {
                    state = S_Q;
                } else if (c == address) {
                    state = S_a;
                }
                break;
            case S_Q: // ?
                if (c == '!') {
                    sendResponse(""); // ?!
                }
                state = S_0;
                break;
            case S_a:
                switch (c) {
                    case '!':
                        sendResponse(""); // a!
                        break;
                    case 'I':
                        state = S_aI;
                        break;
                    case 'A':
                        state = S_aA;
                        break;
                    case 'M':
                        state = S_aM;
                        break;
                    case 'D':
                        state = S_aD;
                        break;
                    default:
                        state = S_0;
                }
                break;
            case S_aI:
                if (c == '!') { // aI!
                    sendResponse("14UOSLOGEOLAGOPU000");
                }
                state = S_0;
                break;
            case S_aA:
                newAddress = (char)c;
                state = S_aAb;
                break;
            case S_aAb:
                if (c == '!') { // aAb!
                    address = newAddress;
                    sendResponse("");
                }
                state = S_0;
                break;
            case S_aM:
                if (c == '!') { // aM!
                    if (as7341_ok) {
                        sendResponse("001a"); // 10 values in 1 second FIXME
                        sensor = AS7341;
                        ok = as7341.readAllChannels();
                        if (! ok) {
                            // TODO Handle error
                        }
                    } else {
                        sendResponse("0000");
                    }
                } else if (c == '1') {
                    state = S_aM1;
                } else if (c == '2') {
                    state = S_aM2;
                } else if (c == '3') {
                    state = S_aM3;
                } else if (c == '4') {
                    state = S_aM4;
                } else if (c == '5') {
                    state = S_aM5;
                } else if (c == '6') {
                    state = S_aM6;
                } else if (c == '7') {
                    state = S_aM7;
                } else if (c >= '8' && c <= '9') {
                    state = S_aMn;
                } else {
                    state = S_0;
                }
                break;
            case S_aM1:
                if (c == '!') { // aM1!
                    if (bme_ok) {
                        sendResponse("0013"); // 3 values in 1 second
                        sensor = BME280;
                        bme_t = bme.readTemperature();
                        bme_p = bme.readPressure() / 100.0F;
                        bme_h = bme.readHumidity();
                    } else {
                        sendResponse("0000");
                    }
                }
                state = S_0;
                break;
            case S_aM2:
                if (c == '!') { // aM2!
                    if (icm_ok) {
                        sendResponse("001a"); // 10 values in 1 second
                        sensor = ICM20X;
                        icm.getEvent(&icm_accel, &icm_gyro, &icm_temp, &icm_mag);
                    } else {
                        sendResponse("0000");
                    }
                }
                state = S_0;
                break;
            case S_aM3:
                if (c == '!') { // aM3!
                    if (mlx_ok) {
                        sendResponse("0012"); // 2 values in 1 second
                        sensor = MLX90614;
                        mlx_o = mlx.readObjectTempC();
                        mlx_a = mlx.readAmbientTempC();
                    } else {
                        sendResponse("0000");
                    }
                }
                state = S_0;
                break;
            case S_aM4:
                if (c == '!') { // aM4!
                    if (sht31_ok) {
                        sendResponse("0012"); // 2 values in 1 second
                        sensor = SHT31;
                        sht_t = sht31.readTemperature();
                        sht_h = sht31.readHumidity();
                    } else {
                        sendResponse("0000");
                    }
                }
                state = S_0;
                break;
            case S_aM5:
                if (c == '!') { // aM5!
                    if (tmp117_ok) {
                        sendResponse("0011"); // 1 value in 1 second
                        sensor = TMP117;
                        tmp117.getEvent(&tmp117_event);
                    } else {
                        sendResponse("0000");
                    }
                }
                state = S_0;
                break;
            case S_aM6:
                if (c == '!') { // aM6!
                    if (veml_ok) {
                        sendResponse("0013"); // 3 values in 1 second
                        sensor = VEML7700;
                        veml_lux = veml.readLux();
                        veml_white = veml.readWhite();
                        veml_als = veml.readALS();
                    } else {
                        sendResponse("0000");
                    }
                }
                state = S_0;
                break;
            case S_aM7:
                if (c == '!') { // aM7!
                    if (vl_ok) {
                        sendResponse("0011"); // 1 value in 1 second
                        sensor = VL53L1;
                        vl.startRanging();
                        vl_distance = vl.getDistance();
                        vl.stopRanging();
                    } else {
                        sendResponse("0000");
                    }
                }
                state = S_0;
                break;
            case S_aMn:
                if (c == '!') { // aMn!
                    sendResponse("0000");
                }
                state = S_0;
                break;
            case S_aD:
                if      (c == '0') { state = S_aD0; }
                else               { state = S_0; }
                break;
            case S_aD0:
                if (c == '!') { // aD0!
                    switch (sensor) {
                        case BME280:
                            if (isnan(sht_t) || isnan(sht_h)) {
                                // TODO What should we send?
                            }
                            sprintf(buffer, "%+.2f%+.2f%+.2f", bme_t, bme_p, bme_h);
                            break;
                        case SHT31:
                            sprintf(buffer, "%+.2f%+.2f", sht_t, sht_h);
                            break;
                        case TMP117:
                            sprintf(buffer, "%+.2f", tmp117_event.temperature);
                            break;
                        case VL53L1:
                            sprintf(buffer, "%+d", vl_distance);
                            break;
                        case MLX90614:
                            sprintf(buffer, "%+.2f%+.2f", mlx_o, mlx_a);
                            break;
                        case VEML7700:
                            sprintf(buffer, "%+.2f%+.2f%+u", veml_lux, veml_white, veml_als);
                            break;
                        case AS7341: // FIXME Too many fields for 1 data command
                            as7341.getAllChannels(as7341_channels);
                            p = buffer;
                            for (int i = 0; i < 10; i++) {
                                p += sprintf(p, "%+u", as7341_channels[i]);
                            }
                            break;
                        case ICM20X:
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
                            break;
                    }
                    sendResponse(buffer);
                }
                state = S_0;
                break;
        }
    }
}
