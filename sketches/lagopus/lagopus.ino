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
#include <Adafruit_BME280.h>
#include <Adafruit_SHT31.h>
#include <Adafruit_TMP117.h>
#include <Wire.h>
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

// States of the state machine for parsing SDI-12 messages
enum states {
    S_0, // initial state (empty string)
    S_Q, // ?
    S_a,
    S_aI,
    S_aA,
    S_aAb,
    S_aM,  // BME280
    S_aM1, // SHT31
    S_aM2, // TMP117
    S_aM3, // VL53L1
    S_aMn, // A SDI-12 sensor must reply to all aMn commands
    S_aD,
    S_aD0,
};

enum sensors {
    BME280,
    SHT31,
    TMP117,
    VL53L1,
};

// Global variables
char address = '5';
enum states state;
enum sensors sensor;

// Sensors and SDI-12 interface
SDI12 sdi12(DATA_PIN); // Create object by which to communicate with the SDI-12 bus on SDIPIN
Adafruit_BME280 bme;
Adafruit_SHT31 sht31 = Adafruit_SHT31();
Adafruit_TMP117 tmp117;
SFEVL53L1X vl53l1(Wire); // XXX Do we use the shutdown/interrupt pins?


void bme280_init()
{
    bool status = bme.begin(0x77);
    if (status) {
        println("BME280\tOK");
    } else {
        println("BME280\tERROR");
    }
}

void sht31_init()
{
    bool status = sht31.begin(0x44); // Set to 0x45 for alternate i2c addr
    if (status) {
        println("SHT31\tOK");
        // Enable the heater for 1s
        sht31.heater(true);
        delay(1000);
        sht31.heater(false);
    } else {
        println("SHT31\tERROR");
    }
}

void tmp117_init()
{
    bool status = tmp117.begin();
    if (status) {
        println("TMP117\tOK");
    } else {
        println("TMP117\tERROR");
    }
}

void vl53l1_init()
{
    bool error = vl53l1.begin();
    if (error) {
        println("VL54L1x\tERROR");
    } else {
        println("VL54L1x\tOK");
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
    println("SDI-12\tOK");

    // Intialize sensors
    Wire.begin(); // XXX Used by the SparkFun libraries
    bme280_init();
    sht31_init();
    tmp117_init();
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

    float bme_t, bme_p, bme_h;
    float sht_t, sht_h;
    sensors_event_t temp; // TMP117
    int distance;
    char buffer[50];

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
                    sendResponse("0013"); // 3 values in 1 second
                    sensor = BME280;
                    bme_t = bme.readTemperature();
                    bme_p = bme.readPressure() / 100.0F;
                    bme_h = bme.readHumidity();
                } else if (c == '1') {
                    state = S_aM1;
                } else if (c == '2') {
                    state = S_aM2;
                } else if (c == '3') {
                    state = S_aM3;
                } else if (c >= '4' && c <= '9') {
                    state = S_aMn;
                } else {
                    state = S_0;
                }
                break;
            case S_aM1:
                if (c == '!') { // aM1!
                    sendResponse("0012"); // 2 values in 1 second
                    sensor = SHT31;
                    sht_t = sht31.readTemperature();
                    sht_h = sht31.readHumidity();
                }
                state = S_0;
                break;
            case S_aM2:
                if (c == '!') { // aM2!
                    sendResponse("0011"); // 1 value in 1 second
                    sensor = TMP117;
                    tmp117.getEvent(&temp);
                }
                state = S_0;
                break;
            case S_aM3:
                if (c == '!') { // aM3!
                    sendResponse("0011"); // 1 value in 1 second
                    sensor = VL53L1;
                    vl53l1.startRanging();
                    distance = vl53l1.getDistance();
                    vl53l1.stopRanging();
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
                            sprintf(buffer, "%+.2f", temp.temperature);
                            break;
                        case VL53L1:
                            sprintf(buffer, "%+d", distance);
                            break;
                    }
                    sendResponse(buffer);
                }
                state = S_0;
                break;
        }
    }
}
