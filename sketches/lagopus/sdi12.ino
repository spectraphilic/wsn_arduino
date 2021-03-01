#include <SDI12.h>


SDI12 sdi12(DATA_PIN);

void sdi12_init()
{
    sdi12.begin();
    delay(500);
    sdi12.forceListen();  // sets SDIPIN as input to prepare for incoming message
    PRINTLN("SDI-12    OK");
}

#ifndef TEST
int sdi12_read()
{
    return sdi12.read();
}

void sdi12_send(const char *response)
{
    sdi12.sendResponse(response);
}


#else
int sdi12_read() {
    static int i = 0;
    char *messages = {
        "?!"
        "5!"
        "5I!"
        "5M!"  "5D0!" "5D1!" // AS7341
        "5M1!" "5D0!"        // BME280
        "5M2!" "5D0!" "5D1!" // ICM20X
        "5M3!" "5D0!"        // MLX90614
        "5M4!" "5D0!"        // SHT31
        "5M5!" "5D0!"        // TMP117
        "5M6!" "5D0!"        // VCNL4040
        "5M7!" "5D0!"        // VEML7700
        "5M8!" "5D0!" "5D1!" // VL53L1
        "5M9!" "5D0!"
    };

    while (i < strlen(messages)) {
        char c = messages[i++];
        PRINT(c);
        return c;
    }

    return '\0';
}

void sdi12_send(const char *response) {
    PRINT(response);
}
#endif
