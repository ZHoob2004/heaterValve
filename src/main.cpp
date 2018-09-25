#include <Arduino.h>
#include <math.h>
#include <HardwareSerial.h>

// Tuning variables - these all need to be adjusted for proper functionality
//const float knobMin = 0.7;
const int knobMin = 70;
//const float knobMax = 1.0;
const int knobMax = 100;

const float vBattDivider = 3.0;     // voltage divider factor, determine experimentally after final assembly
const float vBattMin = 8.0;

const int closeTimeAt10V = 4900;   // time in milliseconds for valve to completely close at 10v
const int closeTimeAt14V = 3400;   // time in milliseconds for valve to completely close at 14v


// pin settings
const int valvePin = 2;       // controls motor ground for valve. needed for all movement
const int relayPin = 3;       // triggers motor reverse relay - ON = close valve
const int knobRawPin = A0;    // potentiometer input for dash knob
const int vBattRawPin = A1;   // voltage divider input for time adjustment


// Global variables and a few constants that shouldn't need adjustment

int knobPosition = 0;     // knob position as a float between 0 and 1, saves some calls to getKnobPosition
int valvePosition = 0;    // valve position as a float between 0 and 1, needed to check for knob movement
float vBatt = 0.0;            // current battery voltage. Refreshes every main loop
unsigned long t = 0;
const float closeTimeM = (closeTimeAt14V-closeTimeAt10V)/(14.0-10.0);
const float closeTimeB = closeTimeAt14V - closeTimeM*14.0;


float getVBatt() {
    //  vBatt = ADC value (0-1023) * 5v max value * voltage divider factor / 1024.0 (10-bit adc, decimal for float conversion)
    return analogRead(vBattRawPin)*5*vBattDivider/1024.0;
}

float getValveTime(float vBatt) {
    //  valveTime is a linear approximation of the time it takes to close the valve at a given voltage
    return closeTimeM*vBatt+closeTimeB;
}


//  Return knob position as a float between 0 and 1
int getKnobPosition() {
    //int knobPosition = analogRead(knobRawPin)*100;
    //knobPosition = knobPosition/1024;
    //return knobPosition;

    return 70;



    //return abs((analogRead(knobRawPin)*100)-knobMin*1024)/(knobMax-knobMin)*1024;
}

// Zero the valve completely closed, then deactivates it. Uses getValveTime(), which uses getVBatt()
void closeValve() {
    digitalWrite(relayPin, HIGH);
    digitalWrite(valvePin, HIGH);
}

void openValve(float position){
    digitalWrite(valvePin, HIGH);
    digitalWrite(relayPin, LOW);
    delay(getValveTime(getVBatt())*position);
    digitalWrite(valvePin, LOW);
}

void setup() {
    Serial.begin(9600);
    pinMode(valvePin, OUTPUT);   
    pinMode(relayPin, OUTPUT);   
}


/*
void loop() {
    Serial.println("getVBatt " + String(getVBatt()));
    valvePosition = getValveTime(getVBatt());
    Serial.println("getValveTime(getVBatt()) " + String(valvePosition));
    Serial.println("getKnobPosition " + String(getKnobPosition()));
    Serial.println("millis " + String(millis()));
    Serial.println("closeValve()");
    closeValve();
    delay(5000);
    Serial.println("openValve()");
    openValve(valvePosition);
}

*/


void loop() {
    vBatt = getVBatt();
    Serial.println("vBatt:" + String(vBatt));
    if (vBatt > vBattMin){
        knobPosition = getKnobPosition();
        Serial.println("knobPosition:" + String(knobPosition));
        if (abs(knobPosition-valvePosition) > 2){         // if knob position differs from expected valve position by >2%, initiate valve repositioning
            valvePosition = knobPosition;
            Serial.println("knob moved:" + String(knobPosition));
            closeValve();
            t = millis();
            Serial.println("closing valve, time t = " + String(t));
            while (millis() < t + 5000 && millis() < t + getValveTime(getVBatt())) {
                Serial.println(String(millis()) + " < t+5000 = " + (String(t+5000)));
                if (abs(valvePosition - getKnobPosition()) > 2) {  // if the knob has moved, reset the counter
                    t = millis();
                    delay(5);
                    valvePosition = getKnobPosition();;
                    Serial.println("knob moved, new t =" + String(t));
                }
                delay(50);
            }
            Serial.println("loop broken, opening to " + String(valvePosition));
            delay(5);
            openValve(valvePosition/100.0);
        }
    }
    delay(10);
}

