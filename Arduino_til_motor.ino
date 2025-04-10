#include <Encoder.h>
#include <PID_v1.h>
#include <Servo.h>

// === PIN DEFINITIONS ===
const int encoderPinA = 2;
const int encoderPinB = 3;
const int enablePin = 5;
const int in1Pin = 7;
const int in2Pin = 6;
const int servoPin = 9;

// === ENCODER SETUP ===
Encoder myEnc(encoderPinA, encoderPinB);

// === PID SETUP ===
double input = 0;
double output = 0;
double setpoint = 0;
double Kp = 1.5, Ki = 0.05, Kd = 0.1;
PID myPID(&input, &output, &setpoint, Kp, Ki, Kd, DIRECT);

// === CONTROL SETTINGS ===
const long maxMotorPos = 20000;
const int camMinY = 30;
const int camMaxY = 440;
const int deadband = 50;
const int shootXMin = 120;
const int shootXMax = 195;
const unsigned long shootCooldown = 1000;  // ms

// === GLOBAL STATE ===
long targetPos = 0;
bool initialized = false;
unsigned long lastUpdateTime = 0;
const int maxNoUpdateDuration = 1000;
unsigned long lastShotTime = 0;
bool shotReady = true;

// === SERVO SETUP ===
Servo kicker;

// === Servo state machine ===
enum ServoState { IDLE, FIRING, RETURNING };
ServoState servoState = IDLE;
unsigned long servoStartTime = 0;
const unsigned long shootDuration = 200;
const unsigned long returnDuration = 300;

// === FUNKTION TIL AT LÆSE LINJER ===
String readSerialLine() {
  static String input = "";
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      String line = input;
      input = "";
      return line;
    } else {
      input += c;
    }
  }
  return "";
}

void shoot() {
  if (shotReady && servoState == IDLE) {
    kicker.write(100);  // skub frem
    servoState = FIRING;
    servoStartTime = millis();
    shotReady = false;
    lastShotTime = millis();
    Serial.println("[SERVO] Skud igangsat!");
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(in1Pin, OUTPUT);
  pinMode(in2Pin, OUTPUT);
  pinMode(enablePin, OUTPUT);

  kicker.attach(servoPin);
  kicker.write(0);  // hvilestilling

  myPID.SetMode(AUTOMATIC);
  myPID.SetOutputLimits(-255, 255);

  Serial.println("Afventer 'START' kommando...");
}

void loop() {
  unsigned long now = millis();

  // === SERVOMASKINE ===
  if (servoState == FIRING && now - servoStartTime > shootDuration) {
    kicker.write(0);  // tilbage til hvilestilling
    servoState = RETURNING;
    servoStartTime = now;
  }
  else if (servoState == RETURNING && now - servoStartTime > returnDuration) {
    servoState = IDLE;
    Serial.println("[SERVO] Klar igen.");
  }

  // === RESET cooldown ===
  if (!shotReady && now - lastShotTime > shootCooldown) {
    shotReady = true;
  }

  // === VENT PÅ START ===
  if (!initialized) {
    String inputStr = readSerialLine();
    if (inputStr == "START") {
      Serial.println("Modtog 'START' - klar til at synkronisere.");
      initialized = true;
    }
    delay(100);
    return;
  }

  // === SYNKRONISERING ===
  if (targetPos == 0) {
    String inputStr = readSerialLine();
    if (inputStr.length() > 0) {
      int spaceIndex = inputStr.indexOf(' ');
      if (spaceIndex > 0) {
        int y = inputStr.substring(spaceIndex + 1).toInt();
        y = constrain(y, camMinY, camMaxY);
        targetPos = map(y, camMinY, camMaxY, 0, maxMotorPos);
        myEnc.write(targetPos);
        lastUpdateTime = now;
        Serial.println("Synkroniseret med boldens position. Starter styring.");
      }
    }
    delay(50);
    return;
  }

  // === MODTAG x og y ===
  String inputStr = readSerialLine();
  if (inputStr.length() > 0) {
    int spaceIndex = inputStr.indexOf(' ');
    if (spaceIndex > 0) {
      int x = inputStr.substring(0, spaceIndex).toInt();
      int y = inputStr.substring(spaceIndex + 1).toInt();

      y = constrain(y, camMinY, camMaxY);
      targetPos = map(y, camMinY, camMaxY, 0, maxMotorPos);
      lastUpdateTime = now;

      if (x >= shootXMin && x <= shootXMax) {
        shoot();  // skyder uden delay
      }

      Serial.print("x: ");
      Serial.print(x);
      Serial.print("  y: ");
      Serial.print(y);
      Serial.print("  => target: ");
      Serial.println(targetPos);
    }
  }

  input = myEnc.read();
  setpoint = targetPos;

  // === STOP MOTOR HVIS INGEN BOLD ===
  if (now - lastUpdateTime > maxNoUpdateDuration) {
    analogWrite(enablePin, 0);
    digitalWrite(in1Pin, LOW);
    digitalWrite(in2Pin, LOW);
    Serial.println("Ingen bold fundet. Motor pauset.");
    delay(100);
    return;
  }

  // === PID MOTORKONTROL ===
  myPID.Compute();

  if (abs(setpoint - input) < deadband) {
    analogWrite(enablePin, 0);
    digitalWrite(in1Pin, LOW);
    digitalWrite(in2Pin, LOW);
  } else {
    int pwm = abs(output);
    pwm = constrain(pwm, 80, 255);
    if (output > 0) {
      digitalWrite(in1Pin, LOW);
      digitalWrite(in2Pin, HIGH);
    } else {
      digitalWrite(in1Pin, HIGH);
      digitalWrite(in2Pin, LOW);
    }
    analogWrite(enablePin, pwm);
  }

  // === DEBUG ===
  Serial.print("Pos: ");
  Serial.print(input);
  Serial.print("  Target: ");
  Serial.print(setpoint);
  Serial.print("  Output: ");
  Serial.print(output);
  Serial.print("  ServoState: ");
  Serial.println(servoState == IDLE ? "IDLE" : (servoState == FIRING ? "FIRING" : "RETURNING"));

  delay(50);
}
