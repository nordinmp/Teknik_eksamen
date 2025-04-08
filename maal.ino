#include <LiquidCrystal.h>

const int rs = 12, en = 11, d4 = 5, d5 = 9, d6 = 3, d7 = 6;
LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

const int BotPin = 4;
const int SpillerPin = 2;
const int MinusSpillerPin = 7;
const int MinusBotPin = 8;
int RobotM = 0;
int SpillerM = 0;

void setup() {
    pinMode(BotPin, INPUT);
    pinMode(SpillerPin, INPUT);
    pinMode(MinusBotPin, INPUT);
    pinMode(MinusSpillerPin, INPUT);
    
    lcd.begin(16, 2);
    updateLCD();
}

void loop() {
    int RobotSignal = digitalRead(BotPin);
    int SpillerSignal = digitalRead(SpillerPin);

    if (RobotSignal == LOW) {
        RobotM++;
        showMessage(RobotM, SpillerM);
        updateLCD();
    }
    
    if (SpillerSignal == LOW) {
        SpillerM++;
        showMessage(RobotM, SpillerM);
        updateLCD();
    }
    
    if (digitalRead(MinusBotPin) == HIGH && RobotM > 0) {
        RobotM--;
        showMessage(RobotM, SpillerM);
        updateLCD();
    }
    
    if (digitalRead(MinusSpillerPin) == HIGH && SpillerM > 0) {
        SpillerM--;
        showMessage(RobotM, SpillerM);
        updateLCD();
    }
}

void updateLCD() {
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("Robot : Spiller");
    lcd.setCursor(5, 1);
    lcd.print(RobotM);
    lcd.print(" : ");
    lcd.print(SpillerM);
}

void showMessage(int robotScore, int spillerScore) {
    lcd.clear();
    lcd.setCursor(1, 0);
    lcd.print("Robot : Spiller");
    lcd.setCursor(5, 1);
    lcd.print(robotScore);
    lcd.print(" : ");
    lcd.print(spillerScore);
    delay(1000);
}
