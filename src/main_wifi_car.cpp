/*
 ============================================================
  ESP32 WiFi RC Car — Smart Control with Obstacle Avoidance
  Hardware: HC-SR04 Ultrasonic, MPU6050, L298N, Encoders
 ============================================================
  
  FEATURES:
  - WiFi-enabled web control interface
  - Dual-mode: Manual control + Autonomous obstacle avoidance
  - Motor balancing with PID correction
  - Encoder-based speed feedback
  - Gyro heading lock for straight driving
  - Servo-based obstacle scanning
  - Real-time telemetry dashboard
  
  REQUIRED LIBRARIES (install via Arduino Library Manager):
  - ESP32Servo by Kevin Harrington
  - MPU6050 by Electronic Cats
  
  CONFIGURATION:
  1. Copy include/config_secrets_template.h to include/config_secrets.h
  2. Edit config_secrets.h with your WiFi credentials
  3. Upload firmware
 ============================================================
*/

#include <WiFi.h>
#include <WebServer.h>
#include <ESPmDNS.h>
#include <Wire.h>
#include <ESP32Servo.h>
#include <MPU6050.h>
#include "config_secrets.h"

// ─────────────────────────────────────────────────────────
// PIN DEFINITIONS (from RC car testing)
// ─────────────────────────────────────────────────────────

// Motor 1 (Left)
const int IN1 = 14;
const int IN2 = 27;
const int ENA = 25; // PWM

// Motor 2 (Right)
const int IN3 = 26;
const int IN4 = 33;
const int ENB = 32; // PWM

// Encoders (HC-89)
const int ENC1_PIN = 4;  // Motor2
const int ENC2_PIN = 5;  // Motor1

// Servo (steering)
const int SERVO_PIN = 13;

// Ultrasonic (HC-SR04)
const int TRIG_PIN = 19;
const int ECHO_PIN = 18;

// I2C (MPU6050)
const int I2C_SDA = 21;
const int I2C_SCL = 22;

// ─────────────────────────────────────────────────────────
// PWM & MOTOR CONFIGURATION
// ─────────────────────────────────────────────────────────

const int pwmFreq = 5000;        // 5kHz for L298N
const int pwmResolution = 8;     // 8-bit (0-255)
const int pwmChannelA = 0;
const int pwmChannelB = 1;

// BALANCED PWM VALUES (tested to deliver same voltage to both motors)
const int BALANCED_LEFT = 180;   // Delivers 6.08V
const int BALANCED_RIGHT = 255;  // Delivers 6.08V

// ─────────────────────────────────────────────────────────
// SERVO SCAN ANGLES (obstacle detection sweep)
// ─────────────────────────────────────────────────────────

#define SCAN_FAR_LEFT    165
#define SCAN_LEFT        135
#define SCAN_FWD_LEFT    105
#define SCAN_CENTER      90
#define SCAN_FWD_RIGHT   75
#define SCAN_RIGHT       45
#define SCAN_FAR_RIGHT   15

// Safety distance thresholds (cm)
#define DIST_STOP        20   // Stop and scan
#define DIST_SLOW        40   // Slow down
#define DIST_BACKUP_ALL  12   // Completely boxed in

// ─────────────────────────────────────────────────────────
// WEB SERVER & SENSORS
// ─────────────────────────────────────────────────────────

WebServer server(80);
MPU6050 mpu;
Servo scanServo;

// ─────────────────────────────────────────────────────────
// STATE VARIABLES
// ─────────────────────────────────────────────────────────

// Motor & encoder state
volatile uint32_t enc1Ticks = 0;
volatile uint32_t enc2Ticks = 0;
int currentSpeedLeft = 0;
int currentSpeedRight = 0;
int motorSpeed = 180;  // Base speed PWM (0-255)

// Control modes
bool obstacleMode = false;
bool fwdLock = false;
bool revLock = false;

// Distance sensing
uint16_t lastDistCm = 999;

// PID & heading correction
#define PID_INTERVAL_MS 40
float pidKp = 2.5f;
float pidKi = 0.08f;
float pidKd = 0.6f;
float pid_integral = 0;
float pid_lastErr = 0;
int pwm_left_base = 0;
int pwm_right_base = 0;
int pwm_left_out = 0;
int pwm_right_out = 0;
unsigned long lastPidMs = 0;
bool pidActive = false;

// MPU6050 heading lock
bool mpuOk = false;
float gyroZ_offset = 0;
float headingErr = 0;
#define GYRO_KP 1.2f

// Obstacle avoidance state machine
enum OAState {
  OA_SCAN,
  OA_FORWARD,
  OA_TURNING,
  OA_BACKUP,
  OA_RESCAN
};

OAState oaState = OA_SCAN;
unsigned long oaTimer = 0;
int oaTurnDir = 0;  // -1=left, 1=right, 0=none
int oaBestAngle = SCAN_CENTER;

// ─────────────────────────────────────────────────────────
// INTERRUPT SERVICE ROUTINES
// ─────────────────────────────────────────────────────────

void IRAM_ATTR isrEnc1() { enc1Ticks++; }
void IRAM_ATTR isrEnc2() { enc2Ticks++; }

// ═════════════════════════════════════════════════════════
// MOTOR CONTROL LAYER
// ═════════════════════════════════════════════════════════

void rawMotors(int leftPWM, int rightPWM) {
  // Left motor (M1)
  if (leftPWM >= 0) {
    digitalWrite(IN1, LOW);
    digitalWrite(IN2, HIGH);  // FORWARD
  } else {
    digitalWrite(IN1, HIGH);
    digitalWrite(IN2, LOW);   // REVERSE
    leftPWM = -leftPWM;
  }

  // Right motor (M2)
  if (rightPWM >= 0) {
    digitalWrite(IN3, LOW);
    digitalWrite(IN4, HIGH);  // FORWARD
  } else {
    digitalWrite(IN3, HIGH);
    digitalWrite(IN4, LOW);   // REVERSE
    rightPWM = -rightPWM;
  }

  pwm_left_out = constrain(leftPWM, 0, 255);
  pwm_right_out = constrain(rightPWM, 0, 255);
  ledcWrite(pwmChannelA, pwm_left_out);
  ledcWrite(pwmChannelB, pwm_right_out);
}

void setMotors(int leftPWM, int rightPWM) {
  pwm_left_base = leftPWM;
  pwm_right_base = rightPWM;
  currentSpeedLeft = abs(leftPWM);
  currentSpeedRight = abs(rightPWM);

  // Enable PID only for straight movement
  pidActive = (leftPWM != 0 && rightPWM != 0 &&
               ((leftPWM > 0) == (rightPWM > 0)) &&
               abs(leftPWM) == abs(rightPWM));

  if (!pidActive) {
    pid_integral = 0;
    pid_lastErr = 0;
    headingErr = 0;
  }

  rawMotors(leftPWM, rightPWM);
}

void stopMotors() {
  pidActive = false;
  pid_integral = 0;
  pid_lastErr = 0;
  headingErr = 0;
  rawMotors(0, 0);
  digitalWrite(IN1, LOW);
  digitalWrite(IN2, LOW);
  digitalWrite(IN3, LOW);
  digitalWrite(IN4, LOW);
  currentSpeedLeft = 0;
  currentSpeedRight = 0;
}

void forward(int s = -1) {
  if (s < 0) s = motorSpeed;
  setMotors(s, s);
}

void backward(int s = -1) {
  if (s < 0) s = motorSpeed;
  setMotors(-s, -s);
}

void turnLeft(int s = -1) {
  if (s < 0) s = motorSpeed;
  setMotors(-s, s);
}

void turnRight(int s = -1) {
  if (s < 0) s = motorSpeed;
  setMotors(s, -s);
}

// ─── PID + Gyro Correction Loop ───
void runPidCorrection() {
  if (!pidActive) return;

  unsigned long now = millis();
  if (now - lastPidMs < PID_INTERVAL_MS) return;

  float dt = (now - lastPidMs) / 1000.0f;
  lastPidMs = now;

  // Read encoder ticks
  noInterrupts();
  uint32_t t1 = enc1Ticks;
  enc1Ticks = 0;
  uint32_t t2 = enc2Ticks;
  enc2Ticks = 0;
  interrupts();

  float encErr = (float)t1 - (float)t2;

  // Gyro heading correction
  float gyroCorr = 0;
  if (mpuOk) {
    int16_t ax, ay, az, gx, gy, gz;
    mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    float gyroZ_dps = (gz - gyroZ_offset) / 131.0f;
    headingErr += gyroZ_dps * dt;
    gyroCorr = headingErr * GYRO_KP;
  }

  float error = encErr + gyroCorr;
  pid_integral = constrain(pid_integral + error * dt, -60.0f, 60.0f);
  float deriv = (error - pid_lastErr) / dt;
  pid_lastErr = error;
  float correction = pidKp * error + pidKi * pid_integral + pidKd * deriv;
  correction = constrain(correction, -50.0f, 50.0f);

  int dir = (pwm_left_base >= 0) ? 1 : -1;
  int newLeft = pwm_left_base - (int)(correction * 0.5f) * dir;
  int newRight = pwm_right_base + (int)(correction * 0.5f) * dir;

  rawMotors(constrain(newLeft, -255, 255),
            constrain(newRight, -255, 255));
}

// ═════════════════════════════════════════════════════════
// SENSOR HELPERS
// ═════════════════════════════════════════════════════════

uint16_t getDistCm() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  if (duration == 0) return 999;

  float distanceCm = duration * 0.0343 / 2.0;
  return (uint16_t)constrain(distanceCm, 0, 999);
}

uint16_t scanAtAngle(int angle) {
  scanServo.write(angle);
  delay(160);  // servo settle + sensor stabilize
  return getDistCm();
}

// Full 7-point obstacle scan
int fullScan(uint16_t results[7]) {
  const int angles[7] = {
    SCAN_FAR_RIGHT, SCAN_RIGHT, SCAN_FWD_RIGHT,
    SCAN_CENTER,
    SCAN_FWD_LEFT, SCAN_LEFT, SCAN_FAR_LEFT
  };

  const float forwardBias[7] = {0.5f, 0.7f, 0.85f, 1.0f, 0.85f, 0.7f, 0.5f};

  float bestScore = -1;
  int bestIdx = 3;

  for (int i = 0; i < 7; i++) {
    results[i] = scanAtAngle(angles[i]);
    float score = min((float)results[i], 200.0f) * forwardBias[i];
    if (score > bestScore) {
      bestScore = score;
      bestIdx = i;
    }
  }

  scanServo.write(SCAN_CENTER);
  return angles[bestIdx];
}

// ═════════════════════════════════════════════════════════
// OBSTACLE AVOIDANCE STATE MACHINE
// ═════════════════════════════════════════════════════════

void runObstacleAvoidance() {
  unsigned long now = millis();

  switch (oaState) {

    case OA_FORWARD: {
      uint16_t d = getDistCm();
      lastDistCm = d;

      if (d > DIST_SLOW) {
        forward(motorSpeed);
      } else if (d > DIST_STOP) {
        int slowSpd = map(d, DIST_STOP, DIST_SLOW, 100, motorSpeed);
        forward(slowSpd);
      } else {
        stopMotors();
        delay(80);
        oaState = OA_SCAN;
      }
      break;
    }

    case OA_SCAN: {
      uint16_t results[7];
      int bestAngle = fullScan(results);
      oaBestAngle = bestAngle;

      bool allClose = true;
      for (int i = 0; i < 7; i++) {
        if (results[i] > DIST_BACKUP_ALL) {
          allClose = false;
          break;
        }
      }

      if (allClose) {
        oaState = OA_BACKUP;
        oaTimer = now;
        backward();
        break;
      }

      if (bestAngle > SCAN_CENTER + 10) {
        oaTurnDir = -1;
        turnLeft();
      } else if (bestAngle < SCAN_CENTER - 10) {
        oaTurnDir = 1;
        turnRight();
      } else {
        oaTurnDir = 0;
      }

      if (oaTurnDir == 0) {
        oaState = OA_FORWARD;
      } else {
        oaState = OA_TURNING;
        oaTimer = now;
      }
      break;
    }

    case OA_TURNING: {
      int angleDiff = abs(oaBestAngle - SCAN_CENTER);
      unsigned long turnMs = map(angleDiff, 0, 90, 0, 420);

      if (now - oaTimer >= turnMs) {
        stopMotors();
        delay(60);
        oaState = OA_RESCAN;
        oaTimer = now;
      }
      break;
    }

    case OA_BACKUP: {
      if (now - oaTimer >= 500) {
        stopMotors();
        delay(60);
        turnRight(motorSpeed);
        delay(750);
        stopMotors();
        delay(60);
        oaState = OA_SCAN;
      }
      break;
    }

    case OA_RESCAN: {
      uint16_t l = scanAtAngle(SCAN_LEFT);
      uint16_t c = scanAtAngle(SCAN_CENTER);
      uint16_t r = scanAtAngle(SCAN_RIGHT);
      lastDistCm = c;

      if (c > DIST_STOP && l > DIST_STOP && r > DIST_STOP) {
        oaState = OA_FORWARD;
      } else {
        oaState = OA_SCAN;
      }
      break;
    }
  }
}

// ═════════════════════════════════════════════════════════
// HTTP HANDLERS
// ═════════════════════════════════════════════════════════

void handleCommand() {
  String cmd = server.arg("c");
  if (server.hasArg("s")) {
    motorSpeed = constrain(server.arg("s").toInt(), 80, 255);
  }

  if (cmd == "mode_manual") {
    obstacleMode = false;
    stopMotors();
    oaState = OA_SCAN;
  } else if (cmd == "mode_obstacle") {
    obstacleMode = true;
    fwdLock = false;
    revLock = false;
    oaState = OA_SCAN;
  } else if (!obstacleMode) {
    if (cmd == "fwd") {
      fwdLock = false;
      revLock = false;
      forward();
    } else if (cmd == "rev") {
      fwdLock = false;
      revLock = false;
      backward();
    } else if (cmd == "left") {
      fwdLock = false;
      revLock = false;
      turnLeft();
    } else if (cmd == "right") {
      fwdLock = false;
      revLock = false;
      turnRight();
    } else if (cmd == "stop") {
      fwdLock = false;
      revLock = false;
      stopMotors();
    } else if (cmd == "lock_fwd") {
      fwdLock = true;
      revLock = false;
      forward();
    } else if (cmd == "lock_rev") {
      fwdLock = false;
      revLock = true;
      backward();
    } else if (cmd == "lock_off") {
      fwdLock = false;
      revLock = false;
      stopMotors();
    }
  }

  server.send(200, "text/plain", "OK");
}

void handleStatus() {
  int16_t ax, ay, az, gx, gy, gz;
  mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

  if (!obstacleMode) {
    scanServo.write(SCAN_CENTER);
    lastDistCm = getDistCm();
  }

  char buf[400];
  snprintf(buf, sizeof(buf),
    "{\"dist\":%u,\"spd\":%d,\"mode\":\"%s\","
    "\"flock\":%s,\"rlock\":%s,"
    "\"ax\":%d,\"ay\":%d,\"az\":%d,"
    "\"e1\":%lu,\"e2\":%lu,"
    "\"pl\":%d,\"pr\":%d,"
    "\"hdg\":%.1f,\"mpu\":%s}",
    lastDistCm, motorSpeed,
    obstacleMode ? "obstacle" : "manual",
    fwdLock ? "true" : "false",
    revLock ? "true" : "false",
    ax / 164, ay / 164, az / 164,
    enc1Ticks, enc2Ticks,
    pwm_left_out, pwm_right_out,
    headingErr, mpuOk ? "true" : "false"
  );

  server.send(200, "application/json", buf);
}

void handleRoot() {
  server.send_P(200, "text/html", (const char*)PROGMEM "<!DOCTYPE html><html><head><meta charset='UTF-8'><meta name='viewport' content='width=device-width,initial-scale=1'><title>ROVER Control</title><style>*{margin:0;padding:0;box-sizing:border-box}body{font-family:'Courier New',monospace;background:#0a0e27;color:#00e5ff;line-height:1.6;padding:20px}h1{text-align:center;margin-bottom:30px;text-shadow:0 0 10px #00e5ff}.container{max-width:600px;margin:0 auto}.panel{background:#111;border:2px solid #00e5ff;border-radius:10px;padding:20px;margin-bottom:20px;box-shadow:0 0 20px rgba(0,229,255,0.2)}.btn{width:100%;padding:15px;margin:5px 0;font-size:16px;font-weight:bold;border:2px solid #00e5ff;background:#0a0e27;color:#00e5ff;cursor:pointer;border-radius:5px;transition:all 0.2s;font-family:'Courier New',monospace}.btn:hover{background:#00e5ff;color:#0a0e27;transform:scale(1.02)}.btn:active{transform:scale(0.98)}.grid{display:grid;grid-template-columns:1fr 1fr;gap:10px}.status{padding:10px;background:#1a1f3a;border:1px solid #00e5ff;border-radius:5px;margin-top:10px;font-size:12px}input[type='range']{width:100%;height:8px;cursor:pointer}label{display:block;margin:10px 0 5px 0;font-weight:bold}</style></head><body><h1>🚗 ROVER Control</h1><div class='container'><div class='panel'><h2>Mode</h2><div class='grid'><button class='btn' onclick='sendCmd(\"mode_manual\")'>📍 Manual</button><button class='btn' onclick='sendCmd(\"mode_obstacle\")'>🤖 Auto Avoid</button></div></div><div class='panel'><h2>Speed</h2><label>Motor Speed: <span id='speedVal'>200</span></label><input type='range' id='speed' min='80' max='255' value='200' oninput='setSpeed(this.value)'></div><div class='panel'><h2>Manual Control</h2><div style='text-align:center'><button class='btn' style='width:120px;margin:5px auto' onclick='sendCmd(\"fwd\")'>⬆️ FWD</button><div class='grid' style='width:300px;margin:10px auto'><button class='btn' onclick='sendCmd(\"left\")'>⬅️ LEFT</button><button class='btn' onclick='sendCmd(\"stop\")'>⏹️ STOP</button><button class='btn' onclick='sendCmd(\"right\")'>➡️ RIGHT</button></div><button class='btn' style='width:120px;margin:5px auto' onclick='sendCmd(\"rev\")'>⬇️ REV</button></div><div class='grid'><button class='btn' onclick='toggleLock(\"fwd\")'>🔒 FWD LOCK</button><button class='btn' onclick='toggleLock(\"rev\")'>🔒 REV LOCK</button></div></div><div class='panel'><h2>Status</h2><div class='status' id='status'>Connecting...</div></div></div><script>let fwdLock=false,revLock=false;function sendCmd(c){fetch('/cmd?c='+c+'&s='+document.getElementById('speed').value);}function setSpeed(v){document.getElementById('speedVal').textContent=v;}function toggleLock(t){fetch('/cmd?c='+t+'_lock&s='+document.getElementById('speed').value);}setInterval(()=>{fetch('/status').then(r=>r.json()).then(d=>{document.getElementById('status').innerHTML='Distance: '+d.dist+' cm<br>Speed: '+d.spd+'<br>Mode: '+d.mode+'<br>PWM L/R: '+d.pl+'/'+d.pr}).catch(e=>{document.getElementById('status').textContent='Connection Lost'});},500);</script></body></html>");
}

// ═════════════════════════════════════════════════════════
// SETUP
// ═════════════════════════════════════════════════════════

void setup() {
  Serial.begin(115200);
  while (!Serial) delay(100);

  Serial.println(F("\n╔══════════════════════════════════╗"));
  Serial.println(F("║  ESP32 WiFi RC Car with OA       ║"));
  Serial.println(F("║  Booting...                      ║"));
  Serial.println(F("╚══════════════════════════════════╝\n"));

  // Motor GPIO setup
  pinMode(IN1, OUTPUT);
  pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT);
  pinMode(IN4, OUTPUT);

  // PWM setup
  ledcSetup(pwmChannelA, pwmFreq, pwmResolution);
  ledcSetup(pwmChannelB, pwmFreq, pwmResolution);
  ledcAttachPin(ENA, pwmChannelA);
  ledcAttachPin(ENB, pwmChannelB);

  stopMotors();

  // Encoders
  pinMode(ENC1_PIN, INPUT_PULLUP);
  pinMode(ENC2_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENC1_PIN), isrEnc1, RISING);
  attachInterrupt(digitalPinToInterrupt(ENC2_PIN), isrEnc2, RISING);

  // Servo
  scanServo.attach(SERVO_PIN, 500, 2400);
  scanServo.write(SCAN_CENTER);
  delay(300);

  // Ultrasonic
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // I2C & MPU6050
  Wire.begin(I2C_SDA, I2C_SCL);
  Wire.setClock(400000);

  mpu.initialize();
  if (!mpu.testConnection()) {
    Serial.println(F("⚠️  MPU6050 NOT FOUND — heading lock disabled"));
    mpuOk = false;
  } else {
    Serial.print(F("📍 Calibrating MPU6050..."));
    mpu.setFullScaleGyroRange(MPU6050_GYRO_FS_250);
    long gzSum = 0;
    for (int i = 0; i < 200; i++) {
      int16_t ax, ay, az, gx, gy, gz;
      mpu.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
      gzSum += gz;
      delay(5);
    }
    gyroZ_offset = gzSum / 200.0f;
    mpuOk = true;
    Serial.printf(" ✓ offset=%.1f\n", gyroZ_offset);
  }

  // WiFi connection
  Serial.print(F("📡 WiFi: Connecting to \""));
  Serial.print(WIFI_SSID);
  Serial.print(F("\"..."));

  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  uint8_t attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 40) {
    delay(500);
    Serial.print('.');
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println(F(" ✓ Connected!"));
    Serial.println(F("\n╔══════════════════════════════════╗"));
    Serial.printf("║  http://%s│\n", WiFi.localIP().toString().c_str());
    Serial.println(F("║  http://rover.local              ║"));
    Serial.println(F("╚══════════════════════════════════╝\n"));

    if (MDNS.begin("rover")) {
      MDNS.addService("http", "tcp", 80);
      Serial.println(F("✓ mDNS 'rover.local' active"));
    }
  } else {
    Serial.println(F(" ✗ FAILED"));
    Serial.println(F("⚠️  Check SSID and password in config_secrets.h"));
  }

  // HTTP routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/cmd", HTTP_GET, handleCommand);
  server.on("/status", HTTP_GET, handleStatus);
  server.enableCORS(true);
  server.begin();

  Serial.println(F("✓ HTTP Server started"));
  Serial.println(F("\n🚀 Ready! Connect to rover.local or mobile app\n"));
}

// ═════════════════════════════════════════════════════════
// MAIN LOOP
// ═════════════════════════════════════════════════════════

void loop() {
  server.handleClient();
  runPidCorrection();

  if (obstacleMode) {
    runObstacleAvoidance();
  } else {
    if (fwdLock) {
      forward();
    } else if (revLock) {
      backward();
    }
  }

  delay(10);
}
