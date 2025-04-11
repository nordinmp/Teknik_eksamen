#include <Encoder.h>
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

// === CONTROL SETTINGS ===
const long camMinY = 30;
const long camMaxY = 440;
const long motorMinPos = 1300;
const long motorMaxPos = 17000;
const int deadband = 25;
const int shootXMin = 140;
const int shootXMax = 225;
const unsigned long shootCooldown = 1000;

// === SERVO SETTINGS ===
const int servoStartPos = 125;
const int kickOffset = 100;

// === GLOBAL STATE ===
long targetPos = 0;
bool initialized = false;
unsigned long lastUpdateTime = 0;
const int maxNoUpdateDuration = 1000;
unsigned long lastShotTime = 0;
bool shotReady = true;

Servo kicker;

// === Servo state machine ===
enum ServoState { IDLE, FIRING, RETURNING };
ServoState servoState = IDLE;
unsigned long servoStartTime = 0;
const unsigned long shootDuration = 400;
const unsigned long returnDuration = 600;

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
    kicker.write(servoStartPos + kickOffset);
    servoState = FIRING;
    servoStartTime = millis();
    shotReady = false;
    lastShotTime = millis();
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(in1Pin, OUTPUT);
  pinMode(in2Pin, OUTPUT);
  pinMode(enablePin, OUTPUT);

  kicker.attach(servoPin);
  kicker.write(servoStartPos);

  Serial.println("Afventer 'START'...");
}

void loop() {
  unsigned long now = millis();

  // Servo styring
  if (servoState == FIRING && now - servoStartTime > shootDuration) {
    kicker.write(servoStartPos);
    servoState = RETURNING;
    servoStartTime = now;
  } else if (servoState == RETURNING && now - servoStartTime > returnDuration) {
    servoState = IDLE;
  }

  if (!shotReady && now - lastShotTime > shootCooldown) {
    shotReady = true;
  }

  if (!initialized) {
    String inputStr = readSerialLine();
    if (inputStr == "START") {
      Serial.println("Modtog 'START' - klar til at synkronisere.");
      initialized = true;
    }
    delay(10);
    return;
  }

  if (targetPos == 0) {
    String inputStr = readSerialLine();
    if (inputStr.length() > 0) {
      int spaceIndex = inputStr.indexOf(' ');
      if (spaceIndex > 0) {
        int y = inputStr.substring(spaceIndex + 1).toInt();
        y = constrain(y, camMinY, camMaxY);
        targetPos = constrain(map(y, camMinY, camMaxY, motorMinPos, motorMaxPos), motorMinPos, motorMaxPos);
        myEnc.write(targetPos);
        lastUpdateTime = now;
      }
    }
    delay(10);
    return;
  }

  String inputStr = readSerialLine();
  if (inputStr.length() > 0) {
    int spaceIndex = inputStr.indexOf(' ');
    if (spaceIndex > 0) {
      int x = inputStr.substring(0, spaceIndex).toInt();
      int y = inputStr.substring(spaceIndex + 1).toInt();

      y = constrain(y, camMinY, camMaxY);
      targetPos = constrain(map(y, camMinY, camMaxY, motorMinPos, motorMaxPos), motorMinPos, motorMaxPos);
      lastUpdateTime = now;

      if (x >= shootXMin && x <= shootXMax) {
        shoot();
      }
    }
  }

  long currentPos = myEnc.read();
  long error = targetPos - currentPos;

  if (abs(error) < deadband) {
    analogWrite(enablePin, 0);
    digitalWrite(in1Pin, LOW);
    digitalWrite(in2Pin, LOW);
  } else {
    digitalWrite(in1Pin, error < 0);
    digitalWrite(in2Pin, error > 0);
    analogWrite(enablePin, 255);
  }

  delay(1);
}
