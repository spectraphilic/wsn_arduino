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

//#define BLINK
//#define TEST
#define DATA_PIN 5   /*!< The pin of the SDI-12 data bus */
#define POWER_PIN -1 /*!< The sensor power pin (or -1 if not switching power) */

#ifdef TEST
    #define PRINT(...) Serial.print(__VA_ARGS__)
    #define PRINTLN(...) Serial.println(__VA_ARGS__)
#else
    #define PRINT(...)
    #define PRINTLN(...)
#endif

// Global objects


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
    NOSENSOR, // special value to signal an error
};

// Global variables
char address = '5';
enum states state;
enum sensors sensor = NOSENSOR;


void setup()
{
#ifdef BLINK
    pinMode(LED_BUILTIN, OUTPUT);
    digitalWrite(LED_BUILTIN, LOW);
#endif

#ifdef TEST
    Serial.begin(9600); // The baudrate of Serial monitor is set in 9600
    while (!Serial); // Waiting for Serial Monitor
#endif

    sdi12_init();
    as7341_init();
    bme280_init();
    icm_init();
    mlx_init();
    sht31_init();
    tmp117_init();
    veml7700_init();
    vl53l1_init();

    PRINTLN("\nTest...\n");
}


int getChar()
{
    while (1) {
        int value = sdi12_read();
        if (value >= 0)
            return value;
    }
}


void sendResponse(const char *msg)
{
    char response[81];
    response[0] = address;
    strcpy(response+1, msg);
    strcat(response, "\r\n");
    sdi12_send(response);
    state = S_0; // Reset automata state
}


void loop()
{
    int c;
    char newAddress;
    char buffer[80];

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
                switch (c) {
                    case '1': state = S_aM1; break;
                    case '2': state = S_aM2; break;
                    case '3': state = S_aM3; break;
                    case '4': state = S_aM4; break;
                    case '5': state = S_aM5; break;
                    case '6': state = S_aM6; break;
                    case '7': state = S_aM7; break;
                    case '8': state = S_aMn; break; // not used
                    case '9': state = S_aMn; break; // not used
                    case '!':
                        as7341_measure();
                    default:
                        state = S_0;
                }
                break;
            case S_aM1:
                if (c == '!')
                    bme280_measure();
                state = S_0;
                break;
            case S_aM2:
                if (c == '!')
                    icm_measure();
                state = S_0;
                break;
            case S_aM3:
                if (c == '!')
                    mlx_measure();
                state = S_0;
                break;
            case S_aM4:
                if (c == '!')
                    sht31_measure();
                state = S_0;
                break;
            case S_aM5:
                if (c == '!')
                    tmp117_measure();
                state = S_0;
                break;
            case S_aM6:
                if (c == '!')
                    veml7700_measure();
                state = S_0;
                break;
            case S_aM7:
                if (c == '!')
                    vl53l1_measure();
                state = S_0;
                break;
            case S_aMn:
                if (c == '!')
                    sendResponse("0000");
                state = S_0;
                break;
            case S_aD:
                if      (c == '0') { state = S_aD0; }
                else               { state = S_0; }
                break;
            case S_aD0:
                if (c == '!') { // aD0!
                    switch (sensor) {
                        case AS7341:
                            as7341_data(buffer);
                            sendResponse(buffer);
                            break;
                        case BME280:
                            bme280_data(buffer);
                            sendResponse(buffer);
                            break;
                        case ICM20X:
                            icm_data(buffer);
                            sendResponse(buffer);
                            break;
                        case MLX90614:
                            mlx_data(buffer);
                            sendResponse(buffer);
                            break;
                        case SHT31:
                            sht31_data(buffer);
                            sendResponse(buffer);
                            break;
                        case TMP117:
                            tmp117_data(buffer);
                            sendResponse(buffer);
                            break;
                        case VEML7700:
                            veml7700_data(buffer);
                            sendResponse(buffer);
                            break;
                        case VL53L1:
                            vl53l1_data(buffer);
                            sendResponse(buffer);
                            break;
                        case NOSENSOR: // No sensor has been selected yet, or it's not available
                            PRINT("\n");
                            break;
                    }
                    sensor = NOSENSOR;
                }
                state = S_0;
                break;
        }
    }
}
