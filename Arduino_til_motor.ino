#include <Encoder.h>
#include <PID_v1.h>

// === PIN DEFINITIONS ===
const int encoderPinA = 2;
const int encoderPinB = 3;
const int enablePin = 5;
const int in1Pin = 7;
const int in2Pin = 6;

// === ENCODER SETUP ===
Encoder myEnc(encoderPinA, encoderPinB);

// === PID SETUP ===
double input = 0;
double output = 0;
double setpoint = 0;
double Kp = 1.5, Ki = 0.05, Kd = 0.1; // Justér disse værdier
PID myPID(&input, &output, &setpoint, Kp, Ki, Kd, DIRECT);

// === CONTROL SETTINGS ===
const long maxMotorPos = 20000;
const int camMinY = 30;
const int camMaxY = 440;

long targetPos = 0;
bool initialized = false;
unsigned long lastUpdateTime = 0;
const int maxNoUpdateDuration = 1000;
const int deadband = 50;  // stadig brugt til at nulstille små udsving

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

  myPID.SetMode(AUTOMATIC);
  myPID.SetOutputLimits(-255, 255); // output kan være negativt (retning)

  Serial.println("Afventer 'START' kommando...");
}

void loop() {
  unsigned long now = millis();

  if (!initialized) {
    String inputStr = readSerialLine();
    if (inputStr == "START") {
      Serial.println("Modtog 'START' – klar til at synkronisere.");
      initialized = true;
    }
    delay(100);
    return;
  }

  if (targetPos == 0) {
    String inputStr = readSerialLine();
    if (inputStr.length() > 0) {
      int spaceIndex = inputStr.indexOf(' ');
      if (spaceIndex > 0) {
        int y = inputStr.substring(spaceIndex + 1).toInt();
        y = constrain(y, camMinY, camMaxY);
        targetPos = map(y, camMinY, camMaxY, 0, maxMotorPos);
        myEnc.write(targetPos); // nulstil position til target
        lastUpdateTime = now;
        Serial.println("Synkroniseret med boldens position. Starter styring.");
      }
    }
    delay(50);
    return;
  }

  String inputStr = readSerialLine();
  if (inputStr.length() > 0) {
    int spaceIndex = inputStr.indexOf(' ');
    if (spaceIndex > 0) {
      int y = inputStr.substring(spaceIndex + 1).toInt();
      y = constrain(y, camMinY, camMaxY);
      targetPos = map(y, camMinY, camMaxY, 0, maxMotorPos);
      lastUpdateTime = now;

      Serial.print("Modtaget y: ");
      Serial.print(y);
      Serial.print(" => Ny target: ");
      Serial.println(targetPos);
    }
  }

  input = myEnc.read();
  setpoint = targetPos;

  if (now - lastUpdateTime > maxNoUpdateDuration) {
    analogWrite(enablePin, 0);
    digitalWrite(in1Pin, LOW);
    digitalWrite(in2Pin, LOW);
    Serial.println("Ingen bold fundet. Motor pauset.");
    delay(100);
    return;
  }

  myPID.Compute(); // Beregn output baseret på input og setpoint

  // Hvis vi er tæt nok på målet, så stop motoren (dødzone)
  if (abs(setpoint - input) < deadband) {
    analogWrite(enablePin, 0);
    digitalWrite(in1Pin, LOW);
    digitalWrite(in2Pin, LOW);
  } else {
    int pwm = abs(output);
    pwm = constrain(pwm, 80, 255); // minimum kraft
    if (output > 0) {
      digitalWrite(in1Pin, LOW);
      digitalWrite(in2Pin, HIGH);
    } else {
      digitalWrite(in1Pin, HIGH);
      digitalWrite(in2Pin, LOW);
    }
    analogWrite(enablePin, pwm);
  }

  Serial.print("Pos: ");
  Serial.print(input);
  Serial.print("  Target: ");
  Serial.print(setpoint);
  Serial.print("  Output: ");
  Serial.println(output);

  delay(50);
}
