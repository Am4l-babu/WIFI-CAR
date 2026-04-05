#ifndef SENSOR_READING_H
#define SENSOR_READING_H

#include <Arduino.h>
#include "pins.h"

// Ultrasonic Sensor Functions
void ultrasonicInit();
float readUltrasonicDistance();  // Returns distance in cm

// Encoder Functions
extern volatile unsigned long encoder1_count;
extern volatile unsigned long encoder2_count;

void encoder1ISR();
void encoder2ISR();
void encoderInit();
unsigned long getEncoder1Count();
unsigned long getEncoder2Count();
void resetEncoders();

// Servo Control Functions
void servoInit();
void setServoAngle(int angle);  // 0-180 degrees
int getServoAngle();

#endif
