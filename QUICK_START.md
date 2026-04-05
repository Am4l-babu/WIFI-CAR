# Quick Start Guide

## 1. Hardware Setup (5 minutes)

Follow [WIRING_CHECKLIST.md](WIRING_CHECKLIST.md) to verify all connections are correct.

**Critical connections:**
- [ ] 12V PSU GND → ESP32 GND (common ground!)
- [ ] All 3.3V sensors powered from ESP32 3.3V pin
- [ ] Motor control IN pins to correct GPIOs
- [ ] PWM pins (GPIO25, GPIO32) to ENA/ENB

## 2. Software Setup (3 minutes)

### Option A: Using VS Code + PlatformIO (Recommended)
```bash
1. Install VS Code
2. Install PlatformIO extension
3. Open this project folder
4. Click Build (checkmark icon) in bottom toolbar
5. Click Upload (arrow icon) to upload to ESP32
6. Click Monitor (power plug icon) to open serial monitor
```

### Option B: Using Arduino IDE
```bash
1. Install Arduino IDE
2. Add ESP32 board: Preferences → Board Manager URLs → https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
3. Install esp32 board from Board Manager
4. Copy files from src/ to main Arduino sketch
5. Upload and open Serial Monitor (Tools → Serial Monitor)
```

### Option C: Using ESP-IDF CLI
```bash
idf.py build
idf.py -p COM3 flash
idf.py -p COM3 monitor
```

## 3. Serial Monitor Setup (1 minute)

**Baud Rate:** 115200
**Port:** COM3 (or your ESP32's COM port)

You should see on startup:
```
========================================
     RC CAR COMPONENT TEST SUITE
========================================

[INIT] Initializing components...
[OK] Motor1 initialized
[OK] Motor2 initialized
...
```

## 4. Testing Components

After startup, you'll see:
```
Available Commands:
  1 - Test Motor 1
  2 - Test Motor 2
  3 - Test Ultrasonic Sensor
  4 - Test MPU6050 IMU
  5 - Test Encoders
  6 - Test GPIO13 Sensor
  7 - Stop all motors

Enter command [1-7]: 
```

### Test Each Component:

#### Test 1: Motor 1
- Type: `1` + Enter
- Creates smooth ramp: forward → stop → reverse → stop
- Reads encoder counts during movement
- **Expected:** Motor should spin smoothly, encoder counts increase
- **Duration:** ~5 seconds

#### Test 2: Motor 2  
- Type: `2` + Enter
- Same ramp test as Motor 1
- **Expected:** Motor should spin smoothly, encoder counts increase
- **Duration:** ~5 seconds

#### Test 3: Ultrasonic Sensor
- Type: `3` + Enter
- Reads distance every 500ms
- **Expected:** 
  - Close range (5-30cm): Shows numbers like 12.5, 18.3
  - Far/No object: "Timeout/No object"
- **Troubleshoot:** Place hand in front of sensor, distance should decrease

#### Test 4: MPU6050
- Type: `4` + Enter
- Reads accelerometer (g) and gyroscope (°/s)
- **Expected:**
  ```
  AccelX | AccelY | AccelZ | GyroX | GyroY | GyroZ
  0.05   | -0.10  | 1.02   | 1.2   | -0.5  | 0.8
  ```
  - Z acceleration ~1.0g (gravity)
  - All gyro values small (at rest)
- **Troubleshoot:** Tilt the board, values should change

#### Test 5: Encoders
- Type: `5` + Enter
- Spins both motors at speed 150 for 5 seconds
- **Expected:**
  ```
  Encoder1 | Encoder2 | Time(ms)
  245      | 242      | 5000
  ```
  - Both should increment
  - Similar counts (balanced motors)
- **Troubleshoot:** Rotate motor shaft by hand, counts should increase

#### Test 6: GPIO13 Sensor
- Type: `6` + Enter
- Reads GPIO13 state every 500ms
- **Expected:** HIGH or LOW state
- **Note:** Purpose unclear - appears to be digital sensor (IR, button, etc.)

#### Stop Motors
- Type: `7` + Enter
- Stops all motors and resets encoders

## 5. Interpreting Results

### Motor Test
```
Speed: 150 | Encoder1: 127
Speed: 200 | Encoder1: 245
```
- Left = motor speed (-255 to +255)
- Right = total pulses counted

### Ultrasonic Test
```
12.5 | VALID
125.3 | VALID
0.0 | Timeout/No object
```
- Distance in cm
- Anything 2-400cm is normal range

### MPU6050 Test
```
AccelX | AccelY | AccelZ | GyroX | GyroY | GyroZ
-0.02  | 0.08   | 0.99   | 0.15  | -0.2  | 0.05
```
- Accel in g (9.8 m/s² = 1g)
- Gyro in degrees/second
- Z accel ~1.0g is normal (gravity)

### Encoder Test
```
Final Encoder1 Count: 1250
Final Encoder2 Count: 1248
```
- Total pulses in 5 seconds
- Similar values = well-balanced motors

## Common Issues

| Problem | Solution |
|---------|----------|
| **Motor doesn't spin** | Check 12V power, verify GPIO pins in code, test with `pio device monitor` |
| **Motor spins but encoder reads 0** | Check encoder GPIO (4/5), verify encoder VCC/GND, check rising edge trigger |
| **Ultrasonic always shows 0** | Check TRIG/ECHO pins, verify 5V supply, clean sensor lens |
| **MPU6050 error reading** | Add pull-up resistors (4.7k) on SDA/SCL, check I2C address (0x68) |
| **Serial monitor shows garbage** | Change baud rate to 115200, check COM port selection |
| **Upload fails** | Verify ESP32 board selected in PlatformIO, check USB cable |

## Next Steps

After all components test successfully:

1. **Create movement routines** - Combine motor control
2. **Obstacle avoidance** - Use ultrasonic + motor control
3. **Stabilization** - Use MPU6050 gyro data
4. **Speed control** - Balance using encoder feedback
5. **Navigation** - Combine all sensors

Good luck! 🚗
