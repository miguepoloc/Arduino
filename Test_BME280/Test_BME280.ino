#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme;

float temperature, humidity, pressure, altitude;

void setup() {
  Serial.begin(115200);
  delay(100);
  bme.begin(0x76);

}
void loop() {
  temperature = bme.readTemperature();
  humidity = bme.readHumidity();
  pressure = bme.readPressure() / 100.0F;
  altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
  Serial.println(temperature);
  Serial.println(humidity);
  Serial.println(pressure);
  Serial.println(altitude);
  delay(1000);
  
}
