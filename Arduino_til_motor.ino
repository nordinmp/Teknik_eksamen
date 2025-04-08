#include <Encoder.h>

// === PIN DEFINITIONS ===
const int encoderPinA = 2;
const int encoderPinB = 3;
const int enablePin = 5;
const int in1Pin = 7;
const int in2Pin = 6;

// === ENCODER SETUP ===
Encoder myEnc(encoderPinA, encoderPinB);

// === CONTROL SETTINGS ===
const int deadband = 50;
const long maxMotorPos = 20000;
const int camMinY = 30;
const int camMaxY = 440;

long targetPos = 0;
bool initialized = false;
unsigned long lastUpdateTime = 0;
const int maxNoUpdateDuration = 1000;

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

void setup() {
  Serial.begin(9600);
  pinMode(in1Pin, OUTPUT);
  pinMode(in2Pin, OUTPUT);
  pinMode(enablePin, OUTPUT);

  Serial.println("Afventer 'START' kommando...");
}

void loop() {
  unsigned long now = millis();

  // === VENT PÅ "START" FRA PYTHON ===
  if (!initialized) {
    String input = readSerialLine();
    if (input == "START") {
      Serial.println("Modtog 'START' – klar til at synkronisere.");
      initialized = true;
    }
    delay(100);
    return;
  }

  // === MODTAG FØRSTE y FOR SYNKRONISERING ===
  if (targetPos == 0) {
    String input = readSerialLine();
    if (input.length() > 0) {
      int spaceIndex = input.indexOf(' ');
      if (spaceIndex > 0) {
        int x = input.substring(0, spaceIndex).toInt();
        int y = input.substring(spaceIndex + 1).toInt();

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

  // === MODTAG NYE KOORDINATER ===
  String input = readSerialLine();
  if (input.length() > 0) {
    int spaceIndex = input.indexOf(' ');
    if (spaceIndex > 0) {
      int x = input.substring(0, spaceIndex).toInt();
      int y = input.substring(spaceIndex + 1).toInt();

      y = constrain(y, camMinY, camMaxY);
      targetPos = map(y, camMinY, camMaxY, 0, maxMotorPos);
      lastUpdateTime = now;

      Serial.print("Modtaget y: ");
      Serial.print(y);
      Serial.print(" => Ny target: ");
      Serial.println(targetPos);
    }
  }

  long motorPos = myEnc.read();
  long error = targetPos - motorPos;

  if (now - lastUpdateTime > maxNoUpdateDuration) {
    analogWrite(enablePin, 0);
    digitalWrite(in1Pin, LOW);
    digitalWrite(in2Pin, LOW);
    Serial.println("Ingen bold fundet. Motor pauset.");
    delay(100);
    return;
  }

  if (abs(error) > 10000) {
    error = (error > 0) ? 10000 : -10000;
  }

  int speed = map(constrain(abs(error), 0, 10000), 0, 10000, 80, 255);

  if (abs(error) > deadband) {
    if (error > 0) {
      digitalWrite(in1Pin, LOW);
      digitalWrite(in2Pin, HIGH);
    } else {
      digitalWrite(in1Pin, HIGH);
      digitalWrite(in2Pin, LOW);
    }
    analogWrite(enablePin, speed);
  } else {
    analogWrite(enablePin, 0);
    digitalWrite(in1Pin, LOW);
    digitalWrite(in2Pin, LOW);
  }

  Serial.print("Pos: ");
  Serial.print(motorPos);
  Serial.print("  Target: ");
  Serial.print(targetPos);
  Serial.print("  Error: ");
  Serial.print(error);
  Serial.print("  Speed: ");
  Serial.println(speed);

  delay(50);
}
