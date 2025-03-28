const  int BotPin = 2; 
const int SpillerPin = 4;
const int MinusSpillerPin = 7;
const int MinusBotPin = 8;
int RobotM = 0;
int SpillerM = 0;

void setup() {
  pinMode(BotPin, INPUT);
  pinMode(SpillerPin, INPUT);
  Serial.begin(9600);
}

void loop() {
  int RobotSignal = digitalRead(BotPin);
  int SpillerSignal = digitalRead(SpillerPin); 

  if (RobotSignal == LOW) {
    RobotM++;
    Serial.println("MÅL TIL ROBOTTEN");
    Serial.print("Score: ");
     Serial.print(RobotM);
     Serial.print(" : ");
     Serial.println(SpillerM);
    delay(1000);
  } 
  if (SpillerSignal == LOW) {
    SpillerM++;
    Serial.println("MÅL TIL SPILLEREN");

     Serial.print("Score: ");
     Serial.print(RobotM);
     Serial.print(" : ");
     Serial.println(SpillerM);
    delay(1000);
  } 
  if (digitalRead(MinusBotPin) == HIGH) {
  RobotM = RobotM - 1;
  Serial.println("Der blev fjernet et mål fra robotten");
  Serial.print("Score: ");
  Serial.print(RobotM);
  Serial.print(" : ");
  Serial.println(SpillerM);
  delay(1000);
  }
  if (digitalRead(MinusSpillerPin) == HIGH) {
  SpillerM = SpillerM - 1;
  Serial.println("Der blev fjernet et mål fra spilleren");
  Serial.print("Score: ");
  Serial.print(RobotM);
  Serial.print(" : ");
  Serial.println(SpillerM);
  delay(1000);
  }
  Serial.println(SpillerSignal);

}
