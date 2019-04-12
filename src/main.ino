#include "Debounce.h"
#include "A6lib.h"

extern "C" { 
    void setup();
    void loop();
};

/* In- and outputs */
#define PIN_HOOK        7
#define PIN_PULSE       8

#define A6_RX 2
#define A6_TX 3

/* Misc constants */
#define ROTARY_HAS_FINISHED_ROTATING_AFTER_MS 100
#define ASSUME_NUMBER_FINISHED_AFTER_MS 6000
#define DEBOUNCE_DELAY 10
#define PRINT_CALL_STATUS_INTERVAL 500

/* States. */
#define  IDLE       0
#define  DIAL       1
#define  ALERTING   2
#define  CONNECTED  3
#define  BUSY       4
#define  RINGING    5

int curstate = IDLE;
int nextstate = IDLE;


Debounce Hook(PIN_HOOK, 30); // Needs a bit more debounce than DEBOUNCE_DELAY

A6lib A6c(A6_RX, A6_TX);

void setup() {
    Serial.begin(9600);
    Serial.println(F("\nI am your phone!"));
    pinMode(PIN_PULSE, INPUT_PULLUP);
    pinMode(PIN_HOOK, INPUT_PULLUP);

    A6c.blockUntilReady(9600);
    int sigStrength = A6c.getSignalStrength();
    Serial.print("Signal strength percentage: ");
    Serial.println(sigStrength);
    A6c.setVol(8);

}

int hookState;

int needToPrint = 0;
int pulseCount;
int lastPulseState = LOW;
int truePulseState = LOW;
long lastPulseChangeTime = 0;
long lastDigitReceivedTime = 0;
long lastCheckCallStatusTime = 0;

String number = "+45";

long currentMillis;

callInfo cinfo;

void ResetState() {
    nextstate = IDLE;
    number = "+45";
    lastDigitReceivedTime = 0;
    needToPrint = 0;
    pulseCount = 0;
    lastPulseChangeTime = 0;
    A6c.hangUp();
}

void Dial() {

    // I'm not using Debounce here because for some odd reason i just doesn't work :(

    /* Check that we have received some digits in the number, and the rotary dial hasn't been dialed for ASSUME_NUMBER_FINISHED_AFTER_MS, then transistion to ALERTING */
    if (lastDigitReceivedTime != 0 && (currentMillis - lastDigitReceivedTime) > ASSUME_NUMBER_FINISHED_AFTER_MS) {
        Serial.println();
        Serial.print(F("Got the number "));
        Serial.println(number);
        nextstate = ALERTING;
        A6c.dial(number);
        Serial.println(F("DIAL > ALERTING"));
        return;
    }

    int pulse = digitalRead(PIN_PULSE);

    // the dial isn’t being dialed, or has just finished being dialed.
    if ((currentMillis - lastPulseChangeTime) > ROTARY_HAS_FINISHED_ROTATING_AFTER_MS) {
        // if it’s only just finished being dialed, we need to send the number down the serial
        // line and reset the pulseCount. We mod the pulseCount by 10 because ‘0’ will send 10 pulses.
        if (needToPrint) {
            Serial.print(pulseCount % 10, DEC);
            number += (int)pulseCount;
            needToPrint = 0;
            pulseCount = 0;
            lastDigitReceivedTime = currentMillis;
        }
    }

    if (pulse != lastPulseState) {
        lastPulseChangeTime = currentMillis;
    }

    if ((currentMillis - lastPulseChangeTime) > DEBOUNCE_DELAY) {
        // debounce - this happens once it’s stablized
        if (pulse != truePulseState) {
            // this means that the switch has either just gone from closed->open or vice versa.
            truePulseState = pulse;
            if (truePulseState == HIGH) {
                // increment the pulseCount of pulses if it’s gone high.
                pulseCount++;
                needToPrint = 1; // we’ll need to print this number (once the dial has finished rotating)
            }
        }
    }
    lastPulseState = pulse;
}

void loop() {
    //Serial.println(curstate);
    currentMillis = millis(); // get snapshot of time for debouncing and timing

    hookState = Hook.read();

    switch (curstate) {

        case IDLE:
            if (hookState == LOW) {
                ResetState();
                nextstate = DIAL;
                Serial.println(F("IDLE > DIAL"));
                break;
            }

            // Currently only check state when not dialing
            if ((currentMillis - lastCheckCallStatusTime) > PRINT_CALL_STATUS_INTERVAL) {
                cinfo = A6c.checkCallStatus();
        //        Serial.print(F("Call state: "));
        //        Serial.println(cinfo.state);
                lastCheckCallStatusTime = currentMillis;
                if (cinfo.state == CALL_INCOMING) {
                    nextstate = RINGING;
                    Serial.println(F("IDLE > RINGING"));
                    break;
                }
            }
            break;


        case DIAL:
            if (hookState == HIGH) {
                ResetState();
                Serial.println(F("DIAL > IDLE"));
                break;
            }
            Dial();
            break;
    
        case ALERTING:
            if (hookState == HIGH) {
                ResetState();
                Serial.println(F("ALERTING > IDLE"));
                break;
            }
            break;
    
        case CONNECTED:
            if (hookState == HIGH) {
                ResetState();
                A6c.hangUp();
                Serial.println(F("CONNECTED > IDLE"));
                break;
            }
            break;
    
        case BUSY:
            Serial.println(F("BUSY"));
            break;
    
        case RINGING:
            //Serial.println(F("RINGING"));
            if (hookState == LOW) {
                Serial.println(F("RINGING > CONNECTED"));
                A6c.answer();
                nextstate = CONNECTED;
                break;
            }

            break;
    
        default:
            Serial.println(F("Error!"));
            ResetState();
            break;
    }

    if (curstate != nextstate) {
        curstate = nextstate;
    }

}