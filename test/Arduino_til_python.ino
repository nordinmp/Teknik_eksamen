void setup() {
  Serial.begin(9600);
}

void loop() {
  if (Serial.available() > 0) {
    int x = Serial.parseInt();
    int y = Serial.parseInt();

    if (Serial.read() == '\n') {
      x += 1;
      y += 1;

      Serial.print(x);
      Serial.print(" ");
      Serial.println(y);
    }
  }
}
