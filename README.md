# RC Car Component Testing Suite

Complete ESP32 firmware for testing all RC car hardware components individually via serial monitor.

## Hardware Configuration

### Power Supply
- **12V PSU** → Motor power (separate from ESP32)
- **GND** → Common ground with ESP32

### Motor 1 (L298N Driver)
- **IN1** → GPIO 14
- **IN2** → GPIO 27  
- **ENA** → GPIO 25 (PWM)
- **OUT1** → Motor1 positive
- **OUT2** → Motor1 negative

### Motor 2 (L298N Driver)
- **IN3** → GPIO 26
- **IN4** → GPIO 33
- **ENB** → GPIO 32 (PWM)
- **OUT3** → Motor2 positive
- **OUT4** → Motor2 negative

### Motor Encoders
- **Motor1 Encoder** → GPIO 4 (digital input, rising edge trigger)
- **Motor2 Encoder** → GPIO 5 (digital input, rising edge trigger)
- **GND** → Common ground
- **VCC** → 3.3V

### Ultrasonic Sensor (HC-SR04)
- **TRIG** → GPIO 19
- **ECHO** → GPIO 18
- **VCC** → 5V
- **GND** → GND

### MPU6050 (I2C IMU)
- **SDA** → GPIO 21 (I2C Data)
- **SCL** → GPIO 22 (I2C Clock)
- **VCC** → 3.3V
- **GND** → GND

### GPIO13 Sensor (5V)
- **Signal** → GPIO 13
- **VCC** → 5V (requires voltage divider if analog)
- **GND** → GND

## Setup Instructions

### 1. Install PlatformIO (VS Code)
```bash
# Install PlatformIO extension in VS Code, or use CLI:
pip install platformio
```

### 2. Build Project
```bash
pio run -e esp32
```

### 3. Upload to ESP32
```bash
pio run -e esp32 -t upload
```

### 4. Monitor Serial Output
```bash
pio device monitor -p COM3 -b 115200
# Or use VS Code's PlatformIO extension (Monitor button)
```

## Testing Menu

Once uploaded, you'll see an interactive menu in the serial monitor:

```
Available Commands:
  1 - Test Motor 1
  2 - Test Motor 2
  3 - Test Ultrasonic Sensor
  4 - Test MPU6050 IMU
  5 - Test Encoders
  6 - Test GPIO13 Sensor
  7 - Stop all motors
```

### Test Details

#### Motor Tests (1, 2)
- Ramps from 0 → 255 (forward)
- Maintains at speed
- Reverses 100 → 255 (backward)
- Ramps down to stop
- Duration: 5 seconds
- Displays encoder count

#### Ultrasonic Test (3)
- Reads distance every 500ms
- Valid range: 2-400 cm
- Displays valid/invalid counts

#### MPU6050 Test (4)
- Reads accelerometer (g) and gyroscope (°/s)
- ±2g and ±250°/s ranges
- Updates every 500ms

#### Encoder Test (5)
- Spins both motors at 150 speed
- Counts pulses for 5 seconds
- Calculates encoder resolution

#### GPIO13 Test (6)
- Reads digital state every 500ms
- Shows HIGH/LOW transitions

## Speed Values

- **0** = Stop
- **1-255** = Forward (1 = slow, 255 = full speed)
- **-1 to -255** = Reverse (-1 = slow, -255 = full speed)

## Troubleshooting

| Issue | Solution |
|-------|----------|
| Motor not spinning | Check power supply (12V), verify IN1/IN2 logic, check ENA PWM signal |
| Encoder not counting | Check encoder connections, ensure rising edge triggers |
| Ultrasonic reads 0 | Check TRIG/ECHO pins, ensure 5V supply, nothing blocking sensor |
| MPU6050 I2C error | Verify SDA/SCL connections, check I2C address (0x68), add pull-up resistors if needed |
| GPIO13 always high/low | Check sensor connection, verify 5V supply |

## Port Configuration

Edit `platformio.ini` if using different COM port:
```ini
monitor_port = COM3  # Change to your port (COM1, COM2, etc.)
```

## Notes

- All PWM runs at 5kHz, 8-bit resolution (0-255)
- Encoder interrupts use rising edge detection
- I2C bus on GPIO 21/22 at standard 100kHz
- Serial baud rate: 115200
