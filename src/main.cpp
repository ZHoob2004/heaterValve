#include <Arduino.h>


/* 
    define pins depending on target platform. Most testing has been done on the Arduino nano, but final design uses an attiny85.
    VALVE_PIN - controls motor ground for valve - must be on for any movement
    RELAY_PIN - triggers motor reverse relay - ON = close valve
    KNOB_PIN - potentiometer input for dash knob, divider between 5v and gnd
    VBATT_PIN - voltage divider input for time adjustment
*/

#ifdef ARDUINO_AVR_NANO
    #define VALVE_PIN 2
    #define RELAY_PIN 3
    #define KNOB_PIN A0
    #define VBATT_PIN A1
#endif

#ifdef ARDUINO_AVR_ATTINYX5
    #define VALVE_PIN 0
    #define RELAY_PIN 1
    #define KNOB_PIN A1
    #define VBATT_PIN A2
#endif

// Tuning variables - these all need to be adjusted for proper functionality
const int knobRawMin = 0;       // min adc value expected for knob, between 0 and 1024
const int knobRawMax = 1024;    // max adc value expected

const int vBattDivider = 3000;      // voltage divider factor, determine experimentally after final assembly
const int vBattMin = 8000;          // minimum voltage for operation (don't bother if below 8000mv)

const int closeTimeAt10V = 4900;    // time in milliseconds for valve to completely close at 10v
const int closeTimeAt14V = 3400;    // time in milliseconds for valve to completely close at 14v





// pin settings from above definitions
const int valvePin = VALVE_PIN;
const int relayPin = RELAY_PIN;
const int knobRawPin = KNOB_PIN;
const int vBattRawPin = VBATT_PIN;


// Global variables and a few constants that shouldn't need adjustment

int knobPosition = 0;     // knob position as an int between 0 and 100, saves some calls to getKnobPosition
int valvePosition = 0;    // valve position as an int between 0 and 100, needed to check for knob movement
int vBatt = 0;            // current battery voltage in mv - Refreshes every main loop
unsigned long t = 0;
const int closeTimeM = (closeTimeAt14V-closeTimeAt10V)/14;
const int closeTimeB = closeTimeAt14V - closeTimeM*14;


int getVBatt() {
    //  vBatt = ADC value (0-1023) * 5v max value * voltage divider factor / 1024
    return analogRead(vBattRawPin)*5*vBattDivider/1024;
}

int getValveTime(int vBatt) {
    //  valveTime is a linear approximation of the time it takes to close the valve at a given voltage
    return closeTimeM*vBatt/1000+closeTimeB;
}


//  Return knob position as a percentage between 0 and 100
int getKnobPosition() {
    //int knobPosition = analogRead(knobRawPin)*100;
    //knobPosition = knobPosition/1024;
    //return knobPosition;

    //return 70;



    //return abs((analogRead(knobRawPin)*100)-knobMin*1024)/(knobMax-knobMin)*1024;
    return abs(analogRead(knobRawPin)-knobRawMin)*100/(knobRawMax-knobRawMin);
}

// Zero the valve completely closed, then deactivates it. Uses getValveTime(), which uses getVBatt()
void closeValve() {
    digitalWrite(relayPin, HIGH);
    digitalWrite(valvePin, HIGH);
}

void openValve(int percentage){
    digitalWrite(valvePin, HIGH);
    digitalWrite(relayPin, LOW);
    delay(getValveTime(getVBatt())*percentage/100);
    digitalWrite(valvePin, LOW);
}

void setup() {
    //Serial.begin(9600);
    pinMode(valvePin, OUTPUT);   
    pinMode(relayPin, OUTPUT);   
}


/*

Serial code for testing variables. Less needed now that main loop is functioning properly. Has not been updated for change to int.
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
    //Serial.println("vBatt:" + String(vBatt));
    if (vBatt > vBattMin){
        knobPosition = getKnobPosition();
        //Serial.println("knobPosition:" + String(knobPosition));
        if (abs(knobPosition-valvePosition) > 2){         // if knob position differs from expected valve position by >2%, initiate valve repositioning
            valvePosition = knobPosition;
            //Serial.println("knob moved:" + String(knobPosition));
            closeValve();
            t = millis();
            //Serial.println("closing valve, time t = " + String(t));
            while (millis() < t + 5000 && millis() < t + getValveTime(getVBatt())) {
                //Serial.println(String(millis()) + " < t+5000 = " + (String(t+5000)));
                if (abs(valvePosition - getKnobPosition()) > 2) {  // if the knob has moved, reset the counter
                    t = millis();
                    delay(5);
                    valvePosition = getKnobPosition();;
                    //Serial.println("knob moved, new t =" + String(t));
                }
                delay(50);
            }
            //Serial.println("loop broken, opening to " + String(valvePosition));
            delay(5);
            openValve(valvePosition);
        }
    }
    delay(10);
}