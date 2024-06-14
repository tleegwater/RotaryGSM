#include <Arduino.h>
#include "Sim800L.h"

#define RX  10
#define TX  11

Sim800L GSM(RX, TX);

enum State {
  STATE_IDLE,
  STATE_RINGING,
  STATE_CALLING
};

State currentState = STATE_IDLE;
const int hookPin = 7;
const int bellPin = 5;
const int rotaryPin = 2;
int rotaryPulseCount = 0;
unsigned long lastRotaryPulse = 0;

bool isHookOff() {
    return digitalRead(hookPin);
}

void ringBell(boolean enable) {
    digitalWrite(bellPin, enable);
}

void onRotaryPulse() {
    if (lastRotaryPulse + 100 < millis()) {
        rotaryPulseCount++;
    } else {
        Serial.println("Rotary pulse count: " + String(rotaryPulseCount +1));
        rotaryPulseCount = 0;
    }
    lastRotaryPulse = millis();
}

void setup() {

    pinMode(hookPin, INPUT_PULLUP);
    pinMode(rotaryPin, INPUT_PULLUP);
    pinMode(bellPin, OUTPUT);
    attachInterrupt(digitalPinToInterrupt(rotaryPin), onRotaryPulse, CHANGE);
    Serial.begin(9600);

    GSM.begin(4800);

    //Serial.println("GET PRODUCT INFO: ");
    //Serial.println(GSM.getProductInfo());
//
    //Serial.println("GET OPERATORS LIST: ");
    //Serial.println(GSM.getOperatorsList());
//
    //Serial.println("GET OPERATOR: ");
    //Serial.println(GSM.getOperator());

}


void loop() {
    switch(currentState) {
        case STATE_IDLE:
            Serial.println("@Idle");
            digitalWrite(bellPin, LOW);
            if (GSM.getCallStatus() == 3) {  //Ringing
                currentState = STATE_RINGING;
            }
            break;

        case STATE_RINGING:
            Serial.println("@Ringing");
            ringBell(true);
            if (isHookOff()) {
                ringBell(false);
                GSM.answerCall();
            }
            if (GSM.getCallStatus() == 4) {  //Call in progress
                currentState = STATE_CALLING;
            }

            break;

        case STATE_CALLING:
            Serial.println("@Calling");
            if (digitalRead(hookPin) == HIGH) {
                GSM.hangoffCall();
                currentState = STATE_IDLE;
            }
            break;
    }
}


