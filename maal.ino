const  int BotPin = 2; 
const int SpillerPin = 4;
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

}
