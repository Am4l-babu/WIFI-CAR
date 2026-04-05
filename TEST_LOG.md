# Test Log Template

Use this log to document your hardware testing results.

## Setup Information

| Item | Details |
|------|---------|
| **Date** | [YYYY-MM-DD] |
| **ESP32 Board** | ESP32-WROOM-32 / DevKit V1 / Other |
| **COM Port** | COM# |
| **Baud Rate** | 115200 |
| **Firmware Version** | v1.0 |
| **12V PSU** | [Brand/Model] |

## Pre-Test Checklist

- [ ] All connections verified with WIRING_CHECKLIST.md
- [ ] No loose wires
- [ ] Power supplies disconnected before final check
- [ ] Firmware uploaded successfully
- [ ] Serial monitor open at 115200 baud

## Component Tests

### Test 1: Motor 1

**Date/Time:** _____________

| Metric | Result | Status |
|--------|--------|--------|
| Motor response to GPIO | [ ] Spins / [ ] Doesn't spin | [ ] OK [ ] FAIL |
| Forward speed range | 0-255 working | [ ] OK [ ] FAIL |
| Reverse working | [ ] Yes / [ ] No | [ ] OK [ ] FAIL |
| Smooth acceleration | [ ] Yes / [ ] No | [ ] OK [ ] FAIL |
| Encoder counting | [ ] Yes / [ ] No | [ ] OK [ ] FAIL |
| **Overall Status** | [ ] PASS / [ ] FAIL | |

**Notes:**
- Motor speed at 100: _____
- Motor speed at 255: _____
- Encoder count after 5s: _____
- Issues observed: _________________

---

### Test 2: Motor 2

**Date/Time:** _____________

| Metric | Result | Status |
|--------|--------|--------|
| Motor response to GPIO | [ ] Spins / [ ] Doesn't spin | [ ] OK [ ] FAIL |
| Forward speed range | 0-255 working | [ ] OK [ ] FAIL |
| Reverse working | [ ] Yes / [ ] No | [ ] OK [ ] FAIL |
| Smooth acceleration | [ ] Yes / [ ] No | [ ] OK [ ] FAIL |
| Encoder counting | [ ] Yes / [ ] No | [ ] OK [ ] FAIL |
| **Overall Status** | [ ] PASS / [ ] FAIL | |

**Notes:**
- Motor speed at 100: _____
- Motor speed at 255: _____
- Encoder count after 5s: _____
- Issues observed: _________________

---

### Test 3: Ultrasonic Sensor (HC-SR04)

**Date/Time:** _____________

| Distance Range | Result | Notes |
|-----------------|--------|-------|
| 5 cm (close) | _____ cm | _____ |
| 15 cm | _____ cm | _____ |
| 30 cm | _____ cm | _____ |
| 100 cm (far) | _____ cm | _____ |
| > 400 cm | _____ | _____ |

**Testing Procedure:**
1. Place object at 5cm from sensor
2. Gradually move object farther
3. Record readings

| Metric | Result | Status |
|--------|--------|--------|
| Responds to nearby object | [ ] Yes / [ ] No | [ ] OK [ ] FAIL |
| Valid range (2-400cm) | [ ] Yes / [ ] No | [ ] OK [ ] FAIL |
| Accuracy ±3cm | [ ] Yes / [ ] No | [ ] OK [ ] FAIL |
| No false triggers | [ ] Yes / [ ] No | [ ] OK [ ] FAIL |
| **Overall Status** | [ ] PASS / [ ] FAIL | |

**Notes:**
- Min distance detected: _____ cm
- Max distance detected: _____ cm
- Issues: _________________

---

### Test 4: MPU6050 IMU Sensor

**Date/Time:** _____________

**At Rest (Board Level):**
| Axis | Min | Max | Average | Expected |
|------|-----|-----|---------|----------|
| Accel X | ____ | ____ | ____ | ~0.0 g |
| Accel Y | ____ | ____ | ____ | ~0.0 g |
| Accel Z | ____ | ____ | ____ | ~1.0 g |
| Gyro X | ____ | ____ | ____ | ~0.0 °/s |
| Gyro Y | ____ | ____ | ____ | ~0.0 °/s |
| Gyro Z | ____ | ____ | ____ | ~0.0 °/s |

**During Movement (Tilt/Shake):**
| Movement | Accel X | Accel Y | Gyro Change | Result |
|----------|---------|---------|-------------|--------|
| Tilt forward | ____ | ____ | ____ | [ ] OK |
| Tilt sideways | ____ | ____ | ____ | [ ] OK |
| Shake vertically | ____ | ____ | ____ | [ ] OK |
| Rotate | ____ | ____ | ____ | [ ] OK |

| Metric | Result | Status |
|--------|--------|--------|
| I2C communication | [ ] Success / [ ] Failed | [ ] OK [ ] FAIL |
| Accelerometer reading | [ ] Yes / [ ] No | [ ] OK [ ] FAIL |
| Gyroscope reading | [ ] Yes / [ ] No | [ ] OK [ ] FAIL |
| Values change with motion | [ ] Yes / [ ] No | [ ] OK [ ] FAIL |
| Z-accel ~1.0g at rest | [ ] Yes / [ ] No | [ ] OK [ ] FAIL |
| **Overall Status** | [ ] PASS / [ ] FAIL | |

**Notes:**
- I2C address found: 0x__
- Issues: _________________

---

### Test 5: Encoders (Motor 1 & Motor 2)

**Date/Time:** _____________

**Motor 1 Encoder Test:**
| Run | Time (s) | Pulses | RPM (est.) | Notes |
|-----|----------|--------|-----------|-------|
| 1 | 5 | ____ | ____ | |
| 2 | 5 | ____ | ____ | |
| 3 | 5 | ____ | ____ | |
| **Average** | 5 | ____ | ____ | |

**Motor 2 Encoder Test:**
| Run | Time (s) | Pulses | RPM (est.) | Notes |
|-----|----------|--------|-----------|-------|
| 1 | 5 | ____ | ____ | |
| 2 | 5 | ____ | ____ | |
| 3 | 5 | ____ | ____ | |
| **Average** | 5 | ____ | ____ | |

**Balance Analysis:**
- Motor1 avg pulses: ______
- Motor2 avg pulses: ______
- Difference: ______%
- Motors balanced: [ ] Yes / [ ] No

| Metric | Result | Status |
|--------|--------|--------|
| Encoder1 counting | [ ] Yes / [ ] No | [ ] OK [ ] FAIL |
| Encoder2 counting | [ ] Yes / [ ] No | [ ] OK [ ] FAIL |
| Counts increase with speed | [ ] Yes / [ ] No | [ ] OK [ ] FAIL |
| Motors reasonably balanced | [ ] Yes / [ ] No | [ ] OK [ ] FAIL |
| **Overall Status** | [ ] PASS / [ ] FAIL | |

**Notes:**
- Gear/wheel rotations per pulse: ______
- Issues: _________________

---

### Test 6: GPIO13 Sensor

**Date/Time:** _____________

**Default State:** [ ] HIGH / [ ] LOW

**State Changes (if applicable):**
| Trigger | Previous | New | Response Time |
|---------|----------|-----|----------------|
| 1 | ____ | ____ | ____ ms |
| 2 | ____ | ____ | ____ ms |
| 3 | ____ | ____ | ____ ms |

| Metric | Result | Status |
|--------|--------|--------|
| GPIO13 readable | [ ] Yes / [ ] No | [ ] OK [ ] FAIL |
| Responds to stimulus | [ ] Yes / [ ] No | [ ] OK [ ] FAIL |
| Stable readings | [ ] Yes / [ ] No | [ ] OK [ ] FAIL |
| **Overall Status** | [ ] PASS / [ ] FAIL | |

**Sensor Type (to be determined):**
- [ ] IR Sensor (obstacle detection)
- [ ] Limit Switch
- [ ] Button
- [ ] Light Sensor
- [ ] Other: _________________

**Notes:**
- Issues: _________________

---

## Overall Summary

| Component | Status |
|-----------|--------|
| Motor 1 | [ ] PASS [ ] FAIL |
| Motor 2 | [ ] PASS [ ] FAIL |
| Ultrasonic | [ ] PASS [ ] FAIL |
| MPU6050 | [ ] PASS [ ] FAIL |
| Encoders | [ ] PASS [ ] FAIL |
| GPIO13 | [ ] PASS [ ] FAIL |

**All Tests Passed:** [ ] YES / [ ] NO

## Issues Found

| Issue | Component | Severity | Solution Attempted | Status |
|-------|-----------|----------|-------------------|--------|
| | | [ ] HIGH / [ ] MEDIUM / [ ] LOW | | [ ] RESOLVED |
| | | [ ] HIGH / [ ] MEDIUM / [ ] LOW | | [ ] RESOLVED |
| | | [ ] HIGH / [ ] MEDIUM / [ ] LOW | | [ ] RESOLVED |

## Next Steps

- [ ] All tests passing - ready for integrated testing
- [ ] Some failures - debug and re-test
- [ ] Major failures - review wiring and connections

**Actions Required:**
1. _________________________________
2. _________________________________
3. _________________________________

---

## Tester Information

**Name:** _________________  
**Date:** _________________  
**Signature:** _________________
