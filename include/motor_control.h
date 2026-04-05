#ifndef MOTOR_CONTROL_H
#define MOTOR_CONTROL_H

#include <Arduino.h>
#include "pins.h"

class MotorController {
private:
    int in1, in2, ena, pwm_channel;
    int current_speed;
    
public:
    MotorController(int _in1, int _in2, int _ena, int _pwm_channel);
    void init();
    void setSpeed(int speed);  // -255 to 255 (negative = reverse)
    void stop();
    int getSpeed();
    void forward(int speed);
    void reverse(int speed);
    void printStatus(const char* motor_name);
};

#endif
