#ifndef PINS_H
#define PINS_H

// Motor 1 Control (L298N)
#define MOTOR1_IN1 14
#define MOTOR1_IN2 27
#define MOTOR1_ENA 25  // PWM
#define MOTOR1_PWM_CHANNEL 0

// Motor 2 Control (L298N)
#define MOTOR2_IN3 26
#define MOTOR2_IN4 33
#define MOTOR2_ENB 32  // PWM
#define MOTOR2_PWM_CHANNEL 1

// Motor Encoders (SWAPPED: Encoder1 is Motor2, Encoder2 is Motor1)
#define ENCODER1_PIN 5  // Motor 2 Encoder
#define ENCODER2_PIN 4  // Motor 1 Encoder

// Ultrasonic Sensor (HC-SR04)
#define ULTRASONIC_TRIG 19
#define ULTRASONIC_ECHO 18

// Servo Control (GPIO 13)
#define SERVO_PIN 13
#define SERVO_PWM_CHANNEL 2

// PWM Configuration
#define PWM_FREQUENCY 5000  // 5kHz
#define PWM_RESOLUTION 8    // 0-255
#define SERVO_FREQUENCY 50  // 50Hz for servo

#endif
