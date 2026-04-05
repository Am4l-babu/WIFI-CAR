# Wiring Checklist

## Before Testing - Hardware Verification

Use this checklist to ensure all connections are correct before uploading the code.

### Power Supply
- [ ] 12V external PSU connected to L298N motors
- [ ] GND from 12V PSU connected to ESP32 GND
- [ ] ESP32 powered via USB (safe 3.3V internal supply)
- [ ] No exposed wires short-circuiting

### Motor 1 Connections
- [ ] IN1 (GPIO14) connected to L298N IN1
- [ ] IN2 (GPIO27) connected to L298N IN2
- [ ] ENA (GPIO25) connected to L298N ENA
- [ ] OUT1 connected to Motor1 positive terminal (+)
- [ ] OUT2 connected to Motor1 negative terminal (-)
- [ ] Motor freespins when powered

### Motor 2 Connections
- [ ] IN3 (GPIO26) connected to L298N IN3
- [ ] IN4 (GPIO33) connected to L298N IN4
- [ ] ENB (GPIO32) connected to L298N ENB
- [ ] OUT3 connected to Motor2 positive terminal (+)
- [ ] OUT4 connected to Motor2 negative terminal (-)
- [ ] Motor freespins when powered

### Encoder Connections
- [ ] Encoder1 DO (output) → GPIO 4
- [ ] Encoder1 GND → ESP32 GND
- [ ] Encoder1 VCC → ESP32 3.3V
- [ ] Encoder2 DO (output) → GPIO 5
- [ ] Encoder2 GND → ESP32 GND
- [ ] Encoder2 VCC → ESP32 3.3V

### Ultrasonic Sensor (HC-SR04)
- [ ] TRIG (GND connection) → GPIO 19
- [ ] ECHO (5V connection) → GPIO 18
- [ ] VCC → 5V (can use external 5V or motor PSU with voltage divider)
- [ ] GND → Common ground

### MPU6050 IMU
- [ ] SDA (data) → GPIO 21
- [ ] SCL (clock) → GPIO 22
- [ ] VCC → ESP32 3.3V
- [ ] GND → ESP32 GND
- [ ] 4.7kΩ pull-up resistors on SDA/SCL (optional but recommended)

### GPIO13 Sensor
- [ ] Signal → GPIO 13
- [ ] VCC → 5V
- [ ] GND → GND

## GPIO Pin Reference Table

| Function | GPIO | Type | Notes |
|----------|------|------|-------|
| Motor1_IN1 | 14 | OUTPUT | Digital |
| Motor1_IN2 | 27 | OUTPUT | Digital |
| Motor1_ENA | 25 | OUTPUT | PWM (ledcSetup) |
| Motor2_IN3 | 26 | OUTPUT | Digital |
| Motor2_IN4 | 33 | OUTPUT | Digital |
| Motor2_ENB | 32 | OUTPUT | PWM (ledcSetup) |
| Encoder1 | 4 | INPUT | Interrupt RisingEdge |
| Encoder2 | 5 | INPUT | Interrupt RisingEdge |
| HC-SR04_TRIG | 19 | OUTPUT | Digital |
| HC-SR04_ECHO | 18 | INPUT | Pulse timing |
| MPU6050_SDA | 21 | I2C_SDA | 100kHz I²C |
| MPU6050_SCL | 22 | I2C_SCL | 100kHz I²C |
| GPIO13_SENSOR | 13 | INPUT | Digital or ADC-capable |

## Safety Notes

⚠️ **Before first power-up:**
1. Double-check motor polarity (+ and - terminals correct)
2. Ensure no loose wires touching each other
3. Test with motors held/secured (don't let them spin freely initially)
4. Start with low speed values (50-100) during first test

⚠️ **During testing:**
- Monitor current draw from 12V PSU
- Check for motor overheating
- Verify no excessive vibration
- Keep hands away from spinning motors

⚠️ **I2C Pull-up Resistors:**
If MPU6050 is not responding:
- Add 4.7kΩ pull-up resistors between SDA-3.3V and SCL-3.3V
- Verify ESP32 I2C port working with logic analyzer if available
