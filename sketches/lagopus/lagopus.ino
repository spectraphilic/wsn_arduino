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

//#define TEST
#define DATA_PIN 5   /*!< The pin of the SDI-12 data bus */
#define RESPONSE_SIZE 55

#ifdef TEST
    #define PRINT(...) Serial.print(__VA_ARGS__)
    #define PRINTLN(...) Serial.println(__VA_ARGS__)
#else
    #define PRINT(...)
    #define PRINTLN(...)
#endif


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
    S_aM6, // VCNL4040
    S_aM7, // VEML7700
    S_aM8, // VL53L1
    S_aMn, // A SDI-12 sensor must reply to all aMn commands
    S_aD,
    S_aD0,
    S_aD1,
    S_aD2,
    S_aD3,
    S_aD4,
    S_aD5,
    S_aD6,
    S_aD7,
    S_aD8,
    S_aD9,
};

// Global variables
char address = '5';
enum states state;


void setup()
{
#ifdef TEST
    Serial.begin(9600); // The baudrate of Serial monitor is set in 9600
    while (!Serial); // Waiting for Serial Monitor
#endif

    sdi12_init();
    as7341_init();
    bme280_init();
    //icm_init();
    mlx_init();
    sht31_init();
    tmp117_init();
    vcnl4040_init();
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

/**
 * Used to split a data response into chunks of max 35 chars (or as defined by
 * RESPONSE_SIZE). It's operation is inspired by strtok:
 *
 * - Pass the response string in the first call, returns the 1st chunk to send.
 * - Pass NULL in the next calls, returns the next chunk.
 * - Returns NULL if there's nothing else to send.
 *
 */
char* get_next_chunk(char *str)
{
    static char chunk[RESPONSE_SIZE+1];
    static char *next;

    char *src = str;
    if (src == NULL) {
        src = next;
        if (src == NULL)
            return NULL;
    }

    char *src_a, *src_p = src;
    char *dst_a, *dst_p = chunk;
    char *src_n = &src[RESPONSE_SIZE];

    src_p = src;
    dst_p = chunk;
    while (1) {
        // Copy
        char c = *src_p;
        *dst_p = c;

        // EOL
        if (c == '\0') {
            next = NULL;
            break;
        }

        // Delimiter
        if (c == '+') {
            src_a = src_p;
            dst_a = dst_p;
        }

        // Overflow
        if (src_p == src_n) {
            *dst_a = '\0';
            next = src_a;
            break;
        }

        // Next
        src_p++;
        dst_p++;
    }

    return chunk;
}


void loop()
{
    int c;
    char newAddress;
    char buffer[100];
    char *str;

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
                    sendResponse("13UOSLOGEOLAGOPU000");
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
                buffer[0] = '\0';
                switch (c) {
                    case '1': state = S_aM1; break;
                    case '2': state = S_aM2; break;
                    case '3': state = S_aM3; break;
                    case '4': state = S_aM4; break;
                    case '5': state = S_aM5; break;
                    case '6': state = S_aM6; break;
                    case '7': state = S_aM7; break;
                    case '8': state = S_aM8; break;
                    case '9': state = S_aMn; break; // not used
                    case '!':
                        as7341_measure(buffer);
                    default:
                        state = S_0;
                }
                break;
            case S_aM1:
                if (c == '!')
                    bme280_measure(buffer);
                state = S_0;
                break;
            case S_aM2:
                if (c == '!')
                    icm_measure(buffer);
                state = S_0;
                break;
            case S_aM3:
                if (c == '!')
                    mlx_measure(buffer);
                state = S_0;
                break;
            case S_aM4:
                if (c == '!')
                    sht31_measure(buffer);
                state = S_0;
                break;
            case S_aM5:
                if (c == '!')
                    tmp117_measure(buffer);
                state = S_0;
                break;
            case S_aM6:
                if (c == '!')
                    vcnl4040_measure(buffer);
                state = S_0;
                break;
            case S_aM7:
                if (c == '!')
                    veml7700_measure(buffer);
                state = S_0;
                break;
            case S_aM8:
                if (c == '!')
                    vl53l1_measure(buffer);
                state = S_0;
                break;
            case S_aMn:
                if (c == '!')
                    sendResponse("0000");
                state = S_0;
                break;
            case S_aD:
                switch (c) {
                    case '0': state = S_aD0; str = buffer; break;
                    case '1': state = S_aD1; str = NULL; break;
                    case '2': state = S_aD2; str = NULL; break;
                    case '3': state = S_aD3; str = NULL; break;
                    case '4': state = S_aD4; str = NULL; break;
                    case '5': state = S_aD5; str = NULL; break;
                    case '6': state = S_aD6; str = NULL; break;
                    case '7': state = S_aD7; str = NULL; break;
                    case '8': state = S_aD8; str = NULL; break;
                    case '9': state = S_aD9; str = NULL; break;
                    default: state = S_0;
                }
                break;
            case S_aD0:
                //PRINTLN();
                //PRINT("=> ");
                //PRINTLN(str);
            case S_aD1:
            case S_aD2:
            case S_aD3:
            case S_aD4:
            case S_aD5:
            case S_aD6:
            case S_aD7:
            case S_aD8:
            case S_aD9:
                if (c == '!') { // aDn!
                    char *chunk = get_next_chunk(str);
                    if (chunk == NULL) {
                        sendResponse("");
                    } else {
                        sendResponse(chunk);
                    }
                }
                state = S_0;
                break;
        }
    }
}
