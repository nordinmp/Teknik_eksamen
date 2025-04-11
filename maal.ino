#include <LiquidCrystal.h>
 
 // Initialiserer LCD med de korrekte ben (RS, EN, D4, D5, D6, D7)
 const int rs = 12, en = 11, d4 = 5, d5 = 9, d6 = 3, d7 = 6;
 LiquidCrystal lcd(rs, en, d4, d5, d6, d7);
 
 // Definerer pins til input fra point- og minusknapper
 const int BotPin = 4;              // Robot scorer point
 const int SpillerPin = 2;          // Spiller scorer point
 const int MinusSpillerPin = 7;     // Trækker point fra spiller
 const int MinusBotPin = 8;         // Trækker point fra robot
 
 // Variabler til at holde styr på point
 int RobotM = 0;
 int SpillerM = 0;
 
 void setup() {
     // Sætter IR og knapper som input
     pinMode(BotPin, INPUT);
     pinMode(SpillerPin, INPUT);
     pinMode(MinusBotPin, INPUT);
     pinMode(MinusSpillerPin, INPUT);
     
     // Initialiserer LCD med 16 kolonner og 2 rækker
     lcd.begin(16, 2);
 }
 
 void loop() {
     // Læser signal fra IR-sensor
     int RobotSignal = digitalRead(BotPin);
     int SpillerSignal = digitalRead(SpillerPin);
 
     // Hvis robotens IR signal brydes (aktiv LOW), tilføjes et point
     if (RobotSignal == LOW) {
         RobotM++;
         showMessage(RobotM, SpillerM);
     }
 
     // Hvis spillerens IR signal brydes (aktiv LOW), tilføjes et point
     if (SpillerSignal == LOW) {
         SpillerM++;
         showMessage(RobotM, SpillerM);
     }
 
     // Hvis minus-knappen for robot trykkes, og scoren er over 0, fjernes et point
     if (digitalRead(MinusBotPin) == HIGH && RobotM > 0) {
         RobotM--;
         showMessage(RobotM, SpillerM);
     }
 
     // Hvis minus-knappen for spiller trykkes, og scoren er over 0, fjrernes et point
     if (digitalRead(MinusSpillerPin) == HIGH && SpillerM > 0) {
         SpillerM--;
         showMessage(RobotM, SpillerM);
     }
 }
 
 // Viser scoren på LCD'en med lidt delay
 void showMessage(int robotScore, int spillerScore) {
     lcd.clear();                        // Rydder displayet
     lcd.setCursor(1, 0);               // Starter i første række
     lcd.print("Robot : Spiller");     // Overskrift
     lcd.setCursor(5, 1);               // Starter midt i anden række
     lcd.print(robotScore);
     lcd.print(" : ");
     lcd.print(spillerScore);
 }
