/*********
  Rui Santos
  Complete project details at https://randomnerdtutorials.com  
*********/

const int analogInPin = A0;  // ESP8266 Analog Pin ADC0 = A0

int sensorValue = 0;  // value read from the pot

void setup() {
  // initialize serial communication at 115200
  Serial.begin(9600);
}

void loop() {
  // read the analog in value
  sensorValue = analogRead(analogInPin);
 
  // print the readings in the Serial Monitor
  Serial.print("sensor = ");
  Serial.println(sensorValue);
  
  delay(1000);
}