int agua;
void setup() {
  Serial.begin(9600);
  pinMode(8, INPUT_PULLUP);
  pinMode(9, OUTPUT);
}

void loop() {
  agua = digitalRead(8);

  if (agua == 0){
    Serial.println("Se detectó agua");
    digitalWrite(9, HIGH);
    delay(100);
  }
  else{
    Serial.println("No hay agua");
    digitalWrite(9, LOW);
    delay(100);
  }
}
