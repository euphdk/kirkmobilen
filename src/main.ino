/* In- and outputs */
#define PIN_PULSE       8

/* Misc constans */
#define NUMBER_LENGTH  8  // Assume it is always a Danish number
#define DIAL_HAS_FINISHED_ROTATING_AFTER_MS 100
#define DEBOUNCE_DELAY 10

/* States. */
#define  IDLE       0
#define  DIAL       1
#define  ALERTING   2
#define  CONNECTED  3
#define  BUSY       4
#define  RINGING    5

int curstate = IDLE;
int nextstate = IDLE;

void setup() {
    Serial.begin(9600);
    Serial.println("\nI am your phone!");
	pinMode(PIN_PULSE, INPUT_PULLUP);
}

int needToPrint = 0;
int pulseCount;
int lastPulseState = LOW;
int truePulseState = LOW;
long lastPulseChangeTime = 0;
int lastDigitReceived = 0;

String number = "";

long currentMillis;


void Dial() {
    int pulse = digitalRead(PIN_PULSE);

    if ((currentMillis - lastPulseChangeTime) > DIAL_HAS_FINISHED_ROTATING_AFTER_MS) {
        // the dial isn’t being dialed, or has just finished being dialed.
        if (needToPrint) {
            // if it’s only just finished being dialed, we need to send the number down the serial
            // line and reset the pulseCount. We mod the pulseCount by 10 because ‘0’ will send 10 pulses.
            Serial.print(pulseCount % 10, DEC);
            number += (int)pulseCount;
            needToPrint = 0;
            pulseCount = 0;

            // Move on if we have reached 8 digits or "112" has been dialed
            if (number.length() == NUMBER_LENGTH || number == "112") {
                Serial.println();
                Serial.print(F("Got the number "));
                Serial.print(number);
                Serial.print(F(", moving to state "));
                nextstate = ALERTING;
                Serial.println(nextstate);
                return;
            }
        }
    }

    if (pulse != lastPulseState) {
        lastPulseChangeTime = millis();
        /* Misc constans */
    }  // Assume it is always a Danish number

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

    switch (curstate) {

        case IDLE:
            Serial.print(F("Idle\n"));
            nextstate = DIAL;
            Serial.print(F("Next state: "));
            Serial.println(nextstate);
            break;


        case DIAL:
            Dial();
            break;
    
        case ALERTING:
            Serial.println(F("ALERTING"));
            Serial.println(number);
            delay(1000);
            break;
    
        case CONNECTED:
            Serial.println(F("CONNECTED"));
            break;
    
        case BUSY:
            Serial.println(F("BUSY"));
            break;
    
        case RINGING:
            Serial.println(F("RINGING"));
            break;
    
        default:
            Serial.println(F("Ehhh.... should'nt have come here... Assume idle. Perhaps you should consider resetting everything."));
            nextstate = IDLE;
            break;
    }

    if (curstate != nextstate) {
        curstate = nextstate;
    }

}