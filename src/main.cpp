#include <Arduino.h>

// Tuning variables - these all need to be adjusted for proper functionality
const int knobRawMin = 0;
const int knobRawMax = 1023;

const float vBattDivider = 3.0;     // voltage divider factor, determine experimentally after final assembly
const float vBattMin = 8.0;

const int closeTimeAt10V = 6000;   // time in milliseconds for valve to completely close at 10v
const int closeTimeAt14V = 3000;   // time in milliseconds for valve to completely close at 14v


// pin settings
const int valvePin = 0;       // controls motor ground for valve. needed for all movement
const int relayPin = 1;       // triggers motor reverse relay - ON = close valve
const int knobRawPin = A1;    // potentiometer input for dash knob
const int vBattRawPin = A2;   // voltage divider input for time adjustment


// Global variables and a few constants that shouldn't need adjustment

float knobPosition = 0.0;     // knob position as a float between 0 and 1, saves some calls to getKnobPosition
float valvePosition = 0.0;    // valve position as a float between 0 and 1, needed to check for knob movement
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
float getKnobPosition() {
    return analogRead(knobRawPin)/1024;
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

}

void loop() {
    vBatt = getVBatt();
    if (vBatt > vBattMin){
        knobPosition = getKnobPosition();
        if (abs(knobPosition-valvePosition) > .02){         // if knob position differs from expected valve position by >2%, initiate valve repositioning
            valvePosition = knobPosition;
            closeValve();
            t = millis();
            while (millis() > t + 5000 && millis() > t + getValveTime(getVBatt())) {
                delay(50);
                if (abs(valvePosition - knobPosition) > .02) {  // if the knob has moved, reset the counter
                    t = millis();
                }
            }
            delay(5);
            openValve(valvePosition);
        }
    }
    delay(5);
}