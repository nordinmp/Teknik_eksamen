#include <PID_v1.h>

// Encoder pins
const int encoderPinA = 2;
const int encoderPinB = 3;

// Motor driver pins
const int enablePin = 5;
const int in1Pin = 6;
const int in2Pin = 7;

// Potentiometer input
const int potPin = A0;

// Encoder tracking
volatile long encoderCount = 0;
int lastEncoded = 0;

// PID setup
double setpoint = 0;
double input = 0;
double output = 0;

double Kp = 1.0, Ki = 0.05, Kd = 0.2;  // PID tuning parametre
PID myPID(&input, &output, &setpoint, Kp, Ki, Kd, DIRECT);

void setup() {
  Serial.begin(9600);

  pinMode(encoderPinA, INPUT_PULLUP);
  pinMode(encoderPinB, INPUT_PULLUP);
  pinMode(enablePin, OUTPUT);
  pinMode(in1Pin, OUTPUT);
  pinMode(in2Pin, OUTPUT);

  attachInterrupt(digitalPinToInterrupt(encoderPinA), updateEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(encoderPinB), updateEncoder, CHANGE);

  myPID.SetMode(AUTOMATIC);
  myPID.SetOutputLimits(-255, 255);
}

void loop() {
  // Læs ønsket position fra potentiometer (map til f.eks. 0–1000)
  setpoint = map(analogRead(potPin), 0, 1023, 0, 5000);
  
  input = encoderCount;
  myPID.Compute();

  setMotor(output);

  // Debug info
  Serial.print("Setpoint: "); Serial.print(setpoint);
  Serial.print(" | Position: "); Serial.print(input);
  Serial.print(" | Output: "); Serial.println(output);

  delay(10);
}

void updateEncoder() {
  int MSB = digitalRead(encoderPinA);
  int LSB = digitalRead(encoderPinB);

  int encoded = (MSB << 1) | LSB;
  int sum = (lastEncoded << 2) | encoded;

  if (sum == 0b1101 || sum == 0b0100 || sum == 0b0010 || sum == 0b1011)
    encoderCount++;
  if (sum == 0b1110 || sum == 0b0111 || sum == 0b0001 || sum == 0b1000)
    encoderCount--;

  lastEncoded = encoded;
}

void setMotor(double pidOutput) {
  if (pidOutput > 0) {
    digitalWrite(in1Pin, HIGH);
    digitalWrite(in2Pin, LOW);
    analogWrite(enablePin, abs(pidOutput));
  } else if (pidOutput < 0) {
    digitalWrite(in1Pin, LOW);
    digitalWrite(in2Pin, HIGH);
    analogWrite(enablePin, abs(pidOutput));
  } else {
    digitalWrite(in1Pin, LOW);
    digitalWrite(in2Pin, LOW);
    analogWrite(enablePin, 0);
  }
}
