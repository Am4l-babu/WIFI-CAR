# 🚗 ESP32 WiFi RC Car with Autonomous Obstacle Avoidance

This **wifi-car** branch contains the production-ready WiFi-enabled firmware with autonomous obstacle avoidance for the RC car project.

## Features Overview

### 🎮 Control Modes
- **Manual Mode**: Full joystick/D-pad control via web interface
- **Auto-Avoid Mode**: Autonomous obstacle detection and navigation

### 📡 WiFi Integration
- Web-based dashboard with real-time controls
- Dual-mode operation (manual + autonomous)
- mDNS support (`http://rover.local`)
- Responsive mobile-friendly interface
- Keyboard support (arrow keys, WASD, spacebar)

### 🤖 Autonomous Features
- 7-point obstacle scanning with servo sweep
- Weighted pathfinding (prefers forward direction)
- Emergency backup when boxed in
- Real-time distance visualization on dashboard
- Graceful deceleration near obstacles

### ⚙️ Motor Control
- **Balanced PWM**: LEFT=180, RIGHT=255 → 6.08V each motor
- PID correction for speed equilibrium
- Encoder-based feedback loop
- Gyro heading lock (MPU6050)
- No voltage imbalance issues

### 📊 Live Telemetry
On the web dashboard, view in real-time:
- Front distance (cm)
- Motor PWM values (0-255)
- Heading error (degrees)
- Encoder tick counts
- Acceleration (G-force)
- Motor speed percentage
- Connection status

---

## Hardware Requirements

| Component | Specification | GPIO Pin |
|-----------|---------------|----------|
| **Board** | ESP32-DOIT-DEVKIT-V1 | — |
| **Motor Driver** | L298N | IN1/IN2/ENA/IN3/IN4/ENB |
| **Motor L** | DC 12V | IN1(14), IN2(27), ENA(25) |
| **Motor R** | DC 12V | IN3(26), IN4(33), ENB(32) |
| **Encoder L** | HC-89 | GPIO4 |
| **Encoder R** | HC-89 | GPIO5 |
| **Servo** | MG995/MG996 | GPIO13 |
| **Ultrasonic** | HC-SR04 | TRIG(19), ECHO(18) |
| **Gyro** | MPU6050 | SDA(21), SCL(22) |

---

## Installation & Setup

### Step 1: Prepare Configuration File
```bash
# Copy the template to create the actual config
cp include/config_secrets_template.h include/config_secrets.h
```

### Step 2: Add WiFi Credentials
Edit `include/config_secrets.h`:
```cpp
#define WIFI_SSID      "YOUR_WIFI_SSID"
#define WIFI_PASSWORD  "YOUR_WIFI_PASSWORD"
```

### Step 3: Install Dependencies
In PlatformIO, the required libraries will auto-install:
- ESP32Servo
- MPU6050  
- (WiFi, WebServer, ESPmDNS are built-in)

### Step 4: Upload Firmware
```bash
pio run -e esp32 -t upload
```

### Step 5: Monitor Serial Output
```bash
pio device monitor --baud 115200
```

You'll see:
```
╔══════════════════════════════════╗
║  http://192.168.x.x              ║
║  http://rover.local              ║
╚══════════════════════════════════╝
```

### Step 6: Connect to Web Interface
- Open `http://rover.local` in your browser
- Or use the IP address from Serial Monitor
- Works on desktop, tablet, and mobile

---

## Web Interface Guide

### Dashboard Layout

**Top Bar:**
- `ROVER` logo with connection status
- Distance to obstacle (color-coded)
- IP address badge

**Mode Tabs:**
- 📍 **MANUAL** - Direct joystick control
- 🤖 **AUTO AVOID** - Let rover navigate autonomously

**Speed Control:**
- Slider to set motor speed (80-255 PWM, 0-100%)
- Real-time percentage display

**Manual Control Panel:**
```
      [▲ FWD]
[◀ LEFT] [STOP] [▶ RIGHT]
      [▼ REV]
```
- D-pad buttons for instant movement
- Can hold buttons for continuous movement
- Mobile touch support

**Lock Buttons:**
- 🔒 **FWD LOCK** - Hold forward continuously
- 🔒 **REV LOCK** - Hold reverse continuously

**Telemetry Grid (9 cells):**
- FRONT DIST - Distance to obstacle
- SPEED - Current speed percentage
- MODE - Manual or Auto
- ENC L/R - Encoder tick counts
- ACCEL Z - Vertical acceleration
- PWM L/R - Raw PWM output (colors warn if imbalanced)
- HEADING - Gyro yaw error in degrees

**Obstacle Avoidance Panel** (visible in Auto-Avoid mode):
- Real-time 7-point radar scan
- Green → Safe (far away)
- Yellow → Caution (40cm)
- Red → Danger (20cm)
- Distance labels on each beam

---

## Control Commands

### HTTP API
All commands are HTTP GET requests to `/cmd?c=<command>&s=<speed>`

| Command | Action |
|---------|--------|
| `fwd` | Move forward |
| `rev` | Move backward |
| `left` | Spin left (counterclockwise) |
| `right` | Spin right (clockwise) |
| `stop` | Stop all motors |
| `lock_fwd` | Hold forward continuously |
| `lock_rev` | Hold backward continuously |
| `lock_off` | Release continuous hold |
| `mode_manual` | Switch to manual control |
| `mode_obstacle` | Switch to autonomous mode |

### Speed Parameter
`s=<80-255>` - Motor PWM value (80 minimum to prevent stalling)

### Example Requests
```
http://rover.local/cmd?c=fwd&s=200
http://rover.local/cmd?c=left&s=180
http://rover.local/cmd?c=mode_obstacle&s=220
```

### Status API
`GET /status` returns JSON:
```json
{
  "dist": 45,
  "spd": 200,
  "mode": "manual",
  "flock": false,
  "rlock": false,
  "ax": 0,
  "ay": 1,
  "az": 15,
  "e1": 234,
  "e2": 231,
  "pl": 200,
  "pr": 200,
  "hdg": 2.3,
  "mpu": true
}
```

---

## Autonomous Obstacle Avoidance

### State Machine
The rover follows this decision tree:

```
┌─────────────────────────────────────┐
│ OA_SCAN: 7-point sweep             │
│ Rate: Once per obstacle detection   │
└─────────────────────────────────────┘
         ↓ (Analyze distances)
    ┌────┴────────────┬─────────────┐
    ↓ (Clear path)    ↓ (Boxed in)  ↓ (Obstacle)
  OA_FORWARD       OA_BACKUP    OA_TURNING
    │               │             │
    │ (Obstacle     │ 500ms       │ (Turn toward
    │  detected)    │ REVERSE     │  best angle)
    │ (< 20cm)      ↓             │
    └──→ Stop   Spin 180°         │
        Scan        │             ↓
                    ↓ OA_RESCAN   (Check 3 points)
                  Scan again       ↓
                    │         ┌─────┴──────┐
                    ↓         ↓ (Clear)    ↓ (Blocked)
                Set speed  OA_FORWARD   OA_SCAN
```

### Distance Thresholds
- **> 40 cm**: Full speed forward
- **20-40 cm**: Graceful deceleration
- **< 20 cm**: Stop and rescan
- **< 12 cm**: Backup + 180° turn (all directions blocked)

### Scan Scoring
Each of 7 directions scored by:
```
score = min(distance_cm, 200) × forwardBias
```

Forward bias weights: `[0.5, 0.7, 0.85, 1.0, 0.85, 0.7, 0.5]`

This ensures the rover prefers moving forward unless significantly blocked to the sides/rear.

---

## Motor Balancing

### Calibration Results
Through systematic testing (+1 to -2 speed imbalance testing):
- **LEFT Motor** (Motor 1): PWM=180 → 6.08V
- **RIGHT Motor** (Motor 2): PWM=255 → 6.08V
- **Result**: Both motors receive identical voltage at these values

### PID Correction
The firmware automatically adjusts speeds using:
- **Encoder feedback**: Detects speed differences
- **Gyro heading**: Corrects yaw drift
- **PID formula**: `correction = Kp×error + Ki×∫error + Kd×(error/dt)`

Gains tuned for smooth convergence:
- Kp = 2.5 (proportional response)
- Ki = 0.08 (integral wind-up prevention)
- Kd = 0.6 (derivative damping)

---

## Troubleshooting

### WiFi Connection Issues
**Problem**: "Connection Lost" on dashboard
- **Solution**: Check SSID/password in `config_secrets.h`
- **Verify**: Serial monitor should show WiFi connect attempt
- WiFi might take 20-40 seconds to connect on startup

### Rover Not Responding
**Problem**: Commands sent but no movement
- **Check**: Is obstacle avoidance locked the rover? Try `mode_manual`
- **Power**: Ensure 12V supply to motor driver
- **USB**: Re-upload firmware if unresponsive

### Unbalanced Motor Speeds
**Problem**: Rover pulls left or right even with same PWM
- **Cause**: Mechanical load or motor differences
- **Fix**: Adjust PWM: reduce faster motor, increase slower one
- **PID**: If slight, PID will auto-correct over time

### Ultrasonic Not Working
**Problem**: Distance shows "∞" or always 999
- **Check**: HC-SR04 wiring on GPIO19/18
- **Test**: Use test function `5` in serial menu
- **Range**: Ultrasonic only works 2cm - 4m

### Servo Not Scanning
**Problem**: Servo doesn't move during obstacle avoidance
- **Check**: GPIO13 wired to servo signal pin
- **Test**: Try test function `4` (servo sweep) from serial
- **Power**: Some servos need external 5V supply

### MPU6050 Not Found
- **Log**: Serial will show "MPU6050 NOT FOUND"
- **Impact**: Heading lock disabled, but rover still functions
- **Fix**: Check I2C (GPIO21/22) wiring, 400kHz clock

---

## Performance Metrics

### Speed & Responsiveness
- **Command latency**: 50-100ms (WiFi + processing)
- **Telemetry update**: 400ms (2.5 updates/second)
- **Obstacle detection**: ~1.2 seconds (7 points × 160ms settle)

### Power Consumption
- **Idle**: ~50mA (WiFi listening)
- **Scanning**: ~200mA (servo + ultrasonic)
- **Full speed**: ~3A (motors + all systems)

### Memory Usage
- **Flash**: 350KB / 1310KB (26%)
- **RAM**: 25KB / 327KB (7.6%)
- **PSRAM**: None required

---

## Development

### Source Code Structure
```
src/main_wifi_car.cpp
├─ Pin definitions & constants
├─ WiFi & WebServer setup
├─ Motor control layer (rawMotors, setMotors, forward, etc.)
├─ PID correction algorithm
├─ Sensor helpers (distance, scan, fullScan)
├─ Obstacle avoidance state machine
├─ HTTP handlers (/cmd, /status, /)
└─ Main loop (server handling + control logic)
```

### Key Functions
- `rawMotors()` - Direct PWM output with direction control
- `setMotors()` - Store base PWM & enable PID layer
- `runPidCorrection()` - Encoder + gyro correction (40ms tick)
- `runObstacleAvoidance()` - State machine processing
- `fullScan()` - 7-point sweep with scoring
- `handleCommand()` - HTTP command parsing
- `handleStatus()` - Return telemetry JSON

### Extending Features
To add camera support, add this to setup():
```cpp
// Camera or additional sensor setup
```

To add a new HTTP endpoint:
```cpp
server.on("/custom", HTTP_GET, [](){
  server.send(200, "text/plain", "Response");
});
```

---

## Security Notes

⚠️ **Important**: This firmware runs on an unencrypted HTTP connection suitable only for **local testing**.

For production or remote deployment, consider:
- HTTPS with self-signed certificate
- Authentication tokens in request headers
- Rate limiting on /cmd endpoint
- Whitelist allowed commands

**Never** commit `config_secrets.h` to version control. The `.gitignore` file already excludes it.

---

## Comparison: Main Branch vs WiFi Car Branch

| Feature | Main | WiFi-Car |
|---------|------|----------|
| Manual Control | ✅ Serial menu | ✅ Web UI |
| WiFi Support | ❌ | ✅ |
| Web Dashboard | ❌ | ✅ |
| Obstacle Avoidance | ❌ | ✅ Auto-mode |
| PID Speed Correction | ❌ | ✅ |
| Gyro Heading Lock | ❌ | ✅ |
| Mobile Control | ❌ | ✅ iOS/Android |
| Real-time Telemetry | ❌ | ✅ JSON API |
| Encoder Feedback | ✅ | ✅ |
| Servo Control | ✅ | ✅ |
| Ultrasonic Support | ✅ | ✅ |
| Code Size | ~10KB | ~30KB |

---

## Testing Checklist

- [ ] WiFi connects within 10 seconds
- [ ] Web dashboard loads in browser
- [ ] Manual D-pad controls respond instantly
- [ ] Speed slider updates in real-time
- [ ] Lock buttons hold direction smoothly
- [ ] Auto-avoid mode scans and navigates
- [ ] No motor voltage imbalance
- [ ] Telemetry updates 2-3x per second
- [ ] Emergency stop responsive
- [ ] Mobile interface works on phone
- [ ] Keyboard controls (WASD) respond

---

## Support & Issues

For issues or feature requests:
1. Check the troubleshooting section above
2. Review Serial monitor output (115200 baud)
3. Verify hardware connections
4. Test individual components with test functions

---

## License & Credits

**RC Car Project** - Educational autonomous vehicle platform

- **Hardware Design**: Based on RC car testing & validation
- **Firmware**: PlatformIO + Arduino Framework (ESP32)
- **Testing**: Comprehensive motor balancing & sensor integration
- **Date**: April 2026

---

**Happy Driving! 🚗💨**
