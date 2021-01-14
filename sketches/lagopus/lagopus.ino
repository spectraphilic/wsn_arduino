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

#define DATA_PIN 5   /*!< The pin of the SDI-12 data bus */
#define POWER_PIN -1 /*!< The sensor power pin (or -1 if not switching power) */

// States of the state machine
enum states {
    INITIAL,
    QUERY, // ?
    ADDRESS, // a
    IDENT, // aI
    CHG, // aA
    CHG_ADDR, // aAb
    START, // aM
    DATA, // aD
    DATA_0, // aD0
};

// Global variables
char address = '5';
int  state;

SDI12 sdi12(DATA_PIN); // Create object by which to communicate with the SDI-12 bus on SDIPIN


void setup()
{
    Serial.begin(9600); // The baudrate of Serial monitor is set in 9600
    while (!Serial); // Waiting for Serial Monitor
    Serial.println("SDI-12 Slave");

    sdi12.begin();
    delay(500);
    sdi12.forceListen();  // sets SDIPIN as input to prepare for incoming message
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
    state = INITIAL; // Reset automata state

//  Serial.print("SEND: ");
//  Serial.println(msg);
}


void loop()
{
    int c;
    char newAddress;

    state = INITIAL;
    while (1) {
        c = getChar();
        switch (state) {
            case INITIAL:
                if (c == '?') {
                    state = QUERY;
                } else if (c == address) {
                    state = ADDRESS;
                }
                break;
            case QUERY: // ?
                if (c == '!') {
                    sendResponse(""); // ?!
                } else {
                    state = INITIAL;
                }
                break;
            case ADDRESS: // a
                switch (c) {
                    case '!':
                        sendResponse(""); // a!
                        break;
                    case 'I':
                        state = IDENT;
                        break;
                    case 'A':
                        state = CHG;
                        break;
                    case 'M':
                        state = START;
                        break;
                    case 'D':
                        state = DATA;
                        break;
                    default:
                        state = INITIAL;
                }
                break;
            case IDENT: // aI
                if (c == '!') { // aI!
                    sendResponse("14UOSLOGEOLAGOPU000");
                }
                state = INITIAL;
                break;
            case CHG: // aA
                newAddress = (char)c;
                state = CHG_ADDR;
                break;
            case CHG_ADDR: // aAb
                if (c == '!') { // aAb!
                    address = newAddress;
                    sendResponse("");
                }
                state = INITIAL;
                break;
            case START: // aM
                if (c == '!') { // aM!
                    sendResponse("0013"); // 3 values in 1 second
                }
                state = INITIAL;
                break;
            case DATA: // aD
                if (c == '0') { // aD0
                    state = DATA_0;
                } else {
                    state = INITIAL;
                }
                break;
            case DATA_0: // aD0
                if (c == '!') { // aD0!
                    sendResponse("0+3.14+2.22+28.00"); // TODO Hardcoded
                }
                state = INITIAL;
                break;
        }
    }
}
