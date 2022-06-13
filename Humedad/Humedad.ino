const int analogInPin = A0;

const int AirValue = 660;   //you need to replace this value with Value_1
const int WaterValue = 190;  //you need to replace this value with Value_2
float soilMoistureValue = 0;
float soilmoisturepercent=0;

void setup() {
  Serial.begin(9600); // open serial port, set the baud rate to 9600 bps
}
void loop() {
  soilMoistureValue = analogRead(analogInPin);  //put Sensor insert into soil
  Serial.println("Valor leido: " + String(soilMoistureValue));
  soilmoisturepercent = map(soilMoistureValue, AirValue, WaterValue, 0, 100);
  Serial.println("Humedad del suelo: " + String(soilmoisturepercent) + " %"); //connect sensor and print the value to serial
  delay(1000);
}
