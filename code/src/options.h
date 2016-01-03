#ifndef __OPTIONS_H_
#define __OPTIONS_H_

#define DEBUG 0
#define DEBUGp(s) if((DEBUG) >= 1) Serial.print((s))
#define DEBUGln(s) if((DEBUG) >= 1) Serial.println((s))

//Use the Piezo buzzer
#define ENABLE_PIEZO

//Use the sensor stick
#define ENABLE_SENSORS

//How long is an animation tick (approximately) (very)
#define TICK_MS 50

//When playing an animation which requires the sensors, what is the (approximate) polling interval
#define SENSE_MS 150

//The I2C address of the ADXL345
#define ADXL345_ADDRESS 0x53

//Activate the LED scanner
#define USE_LED_SCANNER

//The rate of the timer which triggers the LED scanner routine
#define LED_SCANNER_RATE_US 100

//LEDs supported by the scanner
#define NUMLEDS 6

//Total levels of brightness supported by the scanner (range of usable values is therefore 0 to LED_SCANMAX - 1)
#define LED_SCANMAX 8

//The baud rate to use for serial communications
#define BAUD_RATE 115200

//If the magnitude of any axis of the magnetometer goes above this level then stay in sleep mode
#define PACIFICATION_THRESHOLD 2500


//Behaviour
//---------------------------------------------------------------------------------

#define MINSIN8(m) (((m)*60)/8)
#define HOURSIN8(h) (((h)*60*60)/8)
#define ANIMATIONSECS(s) ((1000/TICK_MS) * (s))

//Proper timings
#define BEHAVIOUR_ANIMS_BETWEEN_NOSE_CHANGES random(12, 24)
#define BEHAVIOUR_TIME_BETWEEN_ANIMS_8SECS random(HOURSIN8(3), HOURSIN8(4))
#define BEHAVIOUR_APOPLEXY_THRESHOLD ANIMATIONSECS(120)
#define BEHAVIOUR_APOPLEXY_SLEEP_8SECS HOURSIN8(4/2)

//Demo timings
//#define BEHAVIOUR_ANIMS_BETWEEN_NOSE_CHANGES 300
//#define BEHAVIOUR_TIME_BETWEEN_ANIMS_8SECS 1
//#define BEHAVIOUR_APOPLEXY_THRESHOLD ANIMATIONSECS(120)
//#define BEHAVIOUR_APOPLEXY_SLEEP_8SECS 4

//General behaviour
#define BEHAVIOUR_VIBRATE_THRESHOLD ANIMATIONSECS(13)
#define BEHAVIOUR_BEARING_LEEWAY 45
#define BEHAVIOUR_HAPPY_READINGS_REQUIRED 4


//PIN definitions
//---------------------------------------------------------------------------------

//Sensor stick Vcc pin
#define PIN_SENSOR_POWER A3

//The enable pin for the vibration motor
#define VIBRATE_ENABLE 11

//The plus and minus pins of the Piezo
#define PIEZO_PIN_1 A1
#define PIEZO_PIN_2 A2

//Pin assignments for common colour cathodes
#define RED_PIN 10
#define GREEN_PIN 9
#define BLUE_PIN 8

//The pin assignments for the anode of each tri-colour LED
//i.e. The anode of the LED on face 4 is on the fourth pin listed
//#define LED_ANODE_PINS {7, 6, 5, 4, 3, 2}
#define LED_ANODE_PINS {3, 2, 5, 4, 6, 7}

#endif
