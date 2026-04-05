# Project Structure

```
rc_car/
├── include/                    # Header files
│   ├── pins.h                 # Pin definitions for all components
│   ├── motor_control.h        # Motor control class
│   └── sensor_reading.h       # Sensor reading functions
│
├── src/                       # Source implementation files
│   ├── main.cpp              # Main testing firmware with serial menu
│   ├── motor_control.cpp     # Motor L298N driver implementation
│   └── sensor_reading.cpp    # Sensor reading implementations
│
├── lib/                       # External libraries (if needed)
│
├── platformio.ini            # PlatformIO build configuration
│
├── README.md                 # Full documentation
├── QUICK_START.md           # Quick start guide for first-time setup
├── WIRING_CHECKLIST.md      # Hardware wiring verification
└── PROJECT_STRUCTURE.md     # This file
```

## File Descriptions

### Configuration Files

#### `platformio.ini`
- Build configuration for ESP32 using PlatformIO
- Specifies board type, framework (Arduino), libraries
- COM port and baud rate settings
- Edit `monitor_port = COM3` for your system

### Header Files (include/)

#### `pins.h`
**Purpose:** Centralized pin definition file  
**Contents:**
- All GPIO pin assignments
- PWM channel assignments (0-1 for motors)
- I2C addresses
- Sensor-specific constants (PWM frequency, resolution)

**Why separate:** Makes it easy to remap pins if hardware changes

#### `motor_control.h`
**Purpose:** Motor controller class interface  
**Classes:**
- `MotorController` - controls one L298N motor with:
  - `init()` - initialize GPIO and PWM
  - `setSpeed(speed)` - set speed (-255 to +255)
  - `forward(speed)` - drive forward
  - `reverse(speed)` - drive backward
  - `stop()` - stop motor
  - `printStatus()` - debug output

#### `sensor_reading.h`
**Purpose:** Sensor reading function declarations  
**Functions:**
- **Ultrasonic:** `ultrasonicInit()`, `readUltrasonicDistance()`
- **Encoders:** `encoderInit()`, `getEncoder1Count()`, `getEncoder2Count()`, `resetEncoders()`
- **MPU6050:** `mpu6050Init()`, `readMPU6050()` - reads accel & gyro
- **GPIO13:** `gpio13SensorInit()`, `readGPIO13Sensor()`

### Source Files (src/)

#### `main.cpp`
**Purpose:** Main firmware with interactive testing menu  
**Contents:**
- `setup()` - initializes all components
- `loop()` - waits for serial commands
- Test functions for each component:
  - `testMotor1()` - 5-second acceleration/deceleration ramp
  - `testMotor2()` - same for motor 2
  - `testUltrasonic()` - distance readings
  - `testMPU6050()` - accelerometer & gyroscope
  - `testEncoders()` - spins motors, counts pulses
  - `testGPIO13()` - reads digital sensor
  - `stopAllMotors()` - emergency stop

**Key Features:**
- Interactive menu via serial
- Real-time output during testing
- Automatic statistics (averages, counts)
- Motor ramp profiles for smooth testing

#### `motor_control.cpp`
**Purpose:** Motor controller implementation  
**Key Features:**
- PWM setup using `ledcSetup()` and `ledcAttachPin()`
- Speed ramping with direction changes
- Status display for debugging

#### `sensor_reading.cpp`
**Purpose:** Sensor reading implementations  
**Key Features:**

**Ultrasonic (HC-SR04):**
- Sends 10µs trigger pulse
- Measures echo pulse duration
- Converts to distance (cm)
- Timeout: 30ms (prevents hanging)

**Encoders:**
- Interrupt handlers with `IRAM_ATTR` for ISR
- Rising edge triggers count increment
- Volatile counter for interrupt safety
- Manual reset capability

**MPU6050:**
- I2C communication (Wire library)
- Reads 14 bytes (6 accel + 2 temp + 6 gyro)
- Default ranges: ±2g accel, ±250°/s gyro
- Conversion factors for physical units

**GPIO13:**
- Simple digital read
- Can be modified to analogRead() if needed

### Documentation Files

#### `README.md`
- Complete hardware configuration
- Setup instructions for all environments
- Testing menu reference
- Troubleshooting guide

#### `QUICK_START.md`
- First-time setup walkthrough
- Step-by-step component testing
- Expected output examples
- Common issues and fixes

#### `WIRING_CHECKLIST.md`
- Hardware verification checklist
- GPIO reference table
- Safety notes

## Component Interconnections

```
┌─────────────────────────────────────────┐
│           ESP32 Main Board               │
│  ┌──────────────────────────────────┐   │
│  │    main.cpp (Test Firmware)      │   │
│  └──────┬───────────────────────────┘   │
│         │ Uses                          │
│    ┌────┴─────────────────────────┐     │
│    │                              │     │
│    ▼                              ▼     │
│ ┌─────────────────┐     ┌──────────────────┐
│ │ motor_control.h │     │ sensor_reading.h │
│ │       .cpp      │     │       .cpp       │
│ └────────┬────────┘     └────────┬─────────┘
│          │                       │
│    ┌─────┴─────────┐      ┌──────┴───────────────┐
│    │               │      │                      │
│    ▼               ▼      ▼                      ▼
│  Motor1         Motor2  MPU6050  Ultrasonic  Encoders  GPIO13
│  (GPIO14,      (GPIO26  (I2C)    (GPIO19,    (GPIO4,   (GPIO13)
│   27, 25)       33, 32)           GPIO18)    GPIO5)
```

## Memory and Performance

### RAM Usage
- Motor objects: ~200 bytes each
- Sensor buffers: ~100 bytes  
- Serial buffers: ~512 bytes
- Total: ~1KB (ESP32 has 320KB SRAM)

### CPU Usage
- Main loop: ~1% (mostly idle waiting for serial)
- Motors: PWM auto-generated by hardware
- Interrupts: Lightweight encoder counting

## Extending the Firmware

### Add a New Motor Test
1. Add motor to `pins.h`
2. Create `MotorController` instance in `main.cpp`
3. Call `testMotor()` from menu

### Add a New Sensor
1. Declare function in `sensor_reading.h`
2. Implement in `sensor_reading.cpp`
3. Add test function in `main.cpp`
4. Add menu option

### Change Pin Assignments
1. Edit GPIO numbers in `pins.h`
2. Recompile with `pio run -e esp32`
3. Upload with `pio run -e esp32 -t upload`

Example:
```cpp
// In pins.h - if GPIO14 conflicts with something else:
#define MOTOR1_IN1 12  // Old: 14, New: 12
```

## Compilation Tips

**Fast iteration:**
```bash
# Compile only (no upload)
pio run -e esp32

# Upload to connected ESP32
pio run -e esp32 -t upload

# Everything + monitor
pio run -e esp32 -t uploadfs && pio device monitor -p COM3
```

**Debugging:**
- Add `#define DEBUG 1` to enable verbose serial output
- Check for compiler warnings during build
- Monitor power consumption if erratic behavior occurs

---

That's the complete project structure! Start with QUICK_START.md for first-time setup.
