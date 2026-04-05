/*
 * ESP32 Robot Hardware Validation Suite
 * Features:
 * - Interactive Serial menu for isolated subsystem testing
 * - Hardware timer PWM for precise L298N motor speed control
 * - Hardware interrupts for accurate encoder tick counting
 * - ESP32Servo implementation for jitter-free sweeping
 */

#include <ESP32Servo.h>

// ==========================================
// PIN DEFINITIONS
// ==========================================

// Motor 1 (Left)
const int IN1 = 14;
const int IN2 = 27;
const int ENA = 25; // PWM

// Motor 2 (Right)
const int IN3 = 26;
const int IN4 = 33;
const int ENB = 32; // PWM

// Encoders
const int ENC1_PIN = 4;
const int ENC2_PIN = 5;

// Servo
const int SERVO_PIN = 13;

// Ultrasonic
const int TRIG_PIN = 19;
const int ECHO_PIN = 18;

// ==========================================
// SYSTEM VARIABLES
// ==========================================

Servo testServo;

// PWM Properties for ESP32 Core 2.x (L298N requires ~1kHz-5kHz)
const int pwmFreq = 5000;
const int pwmResolution = 8; // 8-bit resolution (0-255)
const int pwmChannelA = 0;
const int pwmChannelB = 1;

// Balanced PWM Values (both motors get same voltage at these values)
const int BALANCED_LEFT = 180;    // 6.08V
const int BALANCED_RIGHT = 255;   // 6.08V

// Volatile variables for interrupt routines
volatile long encoder1Count = 0;
volatile long encoder2Count = 0;

// Current motor speeds (PWM values 0-255)
int currentSpeedLeft = 0;
int currentSpeedRight = 0;

// ==========================================
// INTERRUPT SERVICE ROUTINES (ISRs)
// ==========================================

void IRAM_ATTR isrEncoder1() {
  encoder1Count++;
}

void IRAM_ATTR isrEncoder2() {
  encoder2Count++;
}

// ==========================================
// FORWARD DECLARATIONS
// ==========================================

void printMenu();
void stopMotors();
void parseAndMove(String input, bool forward);
void printCurrentSpeeds();
void testMotor(int motorNum);
void testBothMotors();
void testServoSweep();
void testUltrasonic();
void testEncoders();

// ==========================================
// SETUP
// ==========================================

void setup() {
  Serial.begin(115200);
  while (!Serial) {;} // Wait for Serial to initialize

  // Initialize Motor Control Pins
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // Configure PWM for Motors (ESP32 Core 2.x API)
  // Note: If using ESP32 Core 3.x, you may use standard analogWrite(ENA, speed) instead
  ledcSetup(pwmChannelA, pwmFreq, pwmResolution);
  ledcAttachPin(ENA, pwmChannelA);
  
  ledcSetup(pwmChannelB, pwmFreq, pwmResolution);
  ledcAttachPin(ENB, pwmChannelB);

  // Ensure motors are stopped initially
  stopMotors();

  // Initialize Encoders
  pinMode(ENC1_PIN, INPUT_PULLUP);
  pinMode(ENC2_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENC1_PIN), isrEncoder1, RISING);
  attachInterrupt(digitalPinToInterrupt(ENC2_PIN), isrEncoder2, RISING);

  // Initialize Ultrasonic Sensor
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Initialize Servo
  // ESP32Servo requires timer allocation
  ESP32PWM::allocateTimer(0);
  testServo.setPeriodHertz(50); // Standard 50hz servo
  testServo.attach(SERVO_PIN, 500, 2400); // Min/Max microseconds

  printMenu();
}

// ==========================================
// MAIN LOOP (MENU SYSTEM)
// ==========================================

void loop() {
  if (Serial.available() > 0) {
    String input = Serial.readStringUntil('\n');
    input.trim();
    input.toLowerCase();
    
    if (input.length() == 0) return;
    
    char command = input.charAt(0);
    
    switch (command) {
      case 'f':
        parseAndMove(input, true);  // true = forward
        break;
      case 'b':
        parseAndMove(input, false);  // false = backward
        break;
      case 's':
        Serial.println("Stopping all motors...");
        stopMotors();
        printMenu();
        break;
      case 'v':
        printCurrentSpeeds();
        printMenu();
        break;
      case '1': testMotor(1); break;
      case '2': testMotor(2); break;
      case '3': testBothMotors(); break;
      case '4': testServoSweep(); break;
      case '5': testUltrasonic(); break;
      case '6': testEncoders(); break;
      case 'h':
        printMenu();
        break;
      default:
        Serial.println("Invalid command. Type 'h' for help.");
        break;
    }
  }
}

// ==========================================
// COMMAND PARSER
// ==========================================

void parseAndMove(String input, bool forward) {
  // Parse format: "f l255 r200" or "b l100 r150"
  // Or simple format: "f" or "b" uses balanced values
  int leftSpeed = 0;
  int rightSpeed = 0;
  
  // Check if using balanced preset (e.g., just "f" or "b")
  if (input.length() == 1) {
    leftSpeed = BALANCED_LEFT;
    rightSpeed = BALANCED_RIGHT;
  } else {
    // Find 'l' position
    int lPos = input.indexOf('l');
    if (lPos == -1) {
      Serial.println("Error: Missing 'l' (e.g., f l255 r255) or use 'f'/'b' for balanced");
      printMenu();
      return;
    }
    
    // Find 'r' position
    int rPos = input.indexOf('r', lPos);
    if (rPos == -1) {
      Serial.println("Error: Missing 'r' (e.g., f l255 r255)");
      printMenu();
      return;
    }
    
    // Extract left speed
    String leftStr = input.substring(lPos + 1, rPos);
    leftStr.trim();
    leftSpeed = leftStr.toInt();
    
    // Extract right speed
    String rightStr = input.substring(rPos + 1);
    rightStr.trim();
    rightSpeed = rightStr.toInt();
  }
  
  // Validate speeds
  leftSpeed = constrain(leftSpeed, 0, 255);
  rightSpeed = constrain(rightSpeed, 0, 255);
  
  // Set motor directions
  if (forward) {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);
    Serial.print("FORWARD: L=");
  } else {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);
    Serial.print("BACKWARD: L=");
  }
  
  // Set motor speeds
  ledcWrite(pwmChannelA, leftSpeed);
  ledcWrite(pwmChannelB, rightSpeed);
  
  // Store current speeds
  currentSpeedLeft = leftSpeed;
  currentSpeedRight = rightSpeed;
  
  Serial.print(leftSpeed);
  Serial.print(" R=");
  Serial.println(rightSpeed);
  printMenu();
}

// ==========================================
// TEST FUNCTIONS
// ==========================================

void printMenu() {
  Serial.println("\n--- ESP32 ROBOT MOTOR CONTROL ---");
  Serial.println("Command Format:");
  Serial.println("  f                     - FORWARD (balanced: L=180, R=255 = 6.08V)");
  Serial.println("  f l<speed> r<speed>  - FORWARD custom (e.g., f l255 r255)");
  Serial.println("  b                     - BACKWARD (balanced: L=180, R=255 = 6.08V)");
  Serial.println("  b l<speed> r<speed>  - BACKWARD custom (e.g., b l200 r255)");
  Serial.println("  s                     - STOP");
  Serial.println("\nMotor Status:");
  Serial.println("  v                     - View current speeds (PWM 0-255)");
  Serial.println("\nTest Commands:");
  Serial.println("  1 - Test Motor 1");
  Serial.println("  2 - Test Motor 2");
  Serial.println("  3 - Test Both Motors");
  Serial.println("  4 - Test Servo Sweep");
  Serial.println("  5 - Test Ultrasonic");
  Serial.println("  6 - Test Encoders");
  Serial.println("  h - Print this menu");
  Serial.println("------------------------------------\n");
}

void stopMotors() {
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  ledcWrite(pwmChannelA, 0);
  ledcWrite(pwmChannelB, 0);
  currentSpeedLeft = 0;
  currentSpeedRight = 0;
}

void printCurrentSpeeds() {
  Serial.println("\n╔════════════════════════════════════╗");
  Serial.println("║    CURRENT MOTOR PWM VALUES         ║");
  Serial.println("╚════════════════════════════════════╝");
  Serial.print("LEFT Motor (Motor 1):  PWM = ");
  Serial.print(currentSpeedLeft);
  Serial.println(" (0-255)");
  Serial.print("RIGHT Motor (Motor 2): PWM = ");
  Serial.print(currentSpeedRight);
  Serial.println(" (0-255)");
  Serial.println("------------------------------------");
}

void testMotor(int motorNum) {
  Serial.print("--- Testing Motor "); Serial.print(motorNum); Serial.println(" ---");
  
  int inA = (motorNum == 1) ? IN2 : IN4;  // SWAPPED: IN2 is now forward, IN1 is backward
  int inB = (motorNum == 1) ? IN1 : IN3;
  int channel = (motorNum == 1) ? pwmChannelA : pwmChannelB;

  Serial.println("Direction: FORWARD (Speed Ramp Up)");
  digitalWrite(inA, HIGH);
  digitalWrite(inB, LOW);
  for (int speed = 0; speed <= 255; speed += 5) {
    ledcWrite(channel, speed);
    delay(50);
  }
  
  Serial.println("Holding Max Speed for 2 seconds...");
  delay(2000);
  
  Serial.println("Stopping for 1 second...");
  ledcWrite(channel, 0);
  delay(1000);

  Serial.println("Direction: REVERSE (Fixed Speed 150)");
  digitalWrite(inA, LOW);
  digitalWrite(inB, HIGH);
  ledcWrite(channel, 150);
  delay(2000);

  Serial.println("Stopping Motor.");
  stopMotors();
  printMenu();
}

void testBothMotors() {
  Serial.println("--- Testing Both Motors ---");
  
  Serial.println("Both FORWARD at 75% speed");
  digitalWrite(IN1, LOW);   // CORRECTED for swapped pins
  digitalWrite(IN2, HIGH);
  digitalWrite(IN3, LOW);   // CORRECTED for swapped pins
  digitalWrite(IN4, HIGH);
  
  ledcWrite(pwmChannelA, 190); // ~75% of 255
  ledcWrite(pwmChannelB, 190);
  delay(3000);

  Serial.println("Stopping...");
  stopMotors();
  printMenu();
}

void testServoSweep() {
  Serial.println("--- Testing Servo ---");
  Serial.println("Sweeping 0 to 180 degrees...");
  
  for (int pos = 0; pos <= 180; pos += 1) {
    testServo.write(pos);
    delay(15);
  }
  
  Serial.println("Sweeping 180 to 0 degrees...");
  for (int pos = 180; pos >= 0; pos -= 1) {
    testServo.write(pos);
    delay(15);
  }
  
  Serial.println("Servo test complete.");
  printMenu();
}

void testUltrasonic() {
  Serial.println("--- Testing Ultrasonic Sensor ---");
  Serial.println("Taking 10 measurements (1 per second):");
  
  for(int i = 0; i < 10; i++) {
    // Clear trigger pin
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    
    // Send 10us pulse to trigger pin
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    
    // Read the echo pin, returns sound wave travel time in microseconds
    long duration = pulseIn(ECHO_PIN, HIGH, 30000); // 30ms timeout (~5 meters max)
    
    if(duration == 0) {
      Serial.println("Reading: Out of range / Error");
    } else {
      // Calculate distance in cm (Speed of sound = 343 m/s or 0.0343 cm/us)
      float distanceCm = duration * 0.0343 / 2.0;
      Serial.print("Reading "); Serial.print(i + 1); Serial.print(": ");
      Serial.print(distanceCm); Serial.println(" cm");
    }
    delay(1000);
  }
  printMenu();
}

void testEncoders() {
  Serial.println("--- Testing Encoders ---");
  Serial.println("Manually spin the motors.");
  Serial.println("Reading ticks for 10 seconds...");
  
  // Reset counters before test
  encoder1Count = 0;
  encoder2Count = 0;

  unsigned long startTime = millis();
  while (millis() - startTime < 10000) {
    Serial.print("Encoder 1 (GPIO 4): "); Serial.print(encoder1Count);
    Serial.print(" | Encoder 2 (GPIO 5): "); Serial.println(encoder2Count);
    delay(500); // Print twice a second
  }
  
  Serial.println("Encoder test complete.");
  printMenu();
}
