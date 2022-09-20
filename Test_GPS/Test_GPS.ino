/*some obs:
 *  * There are three serial ports on the ESP known as U0UXD, U1UXD and U2UXD.

   U0UXD is used to communicate with the ESP32 for programming and during reset/boot.
   U1UXD is unused and can be used for your projects. Some boards use this port for SPI Flash access though
   U2UXD is unused and can be used for your projects.
   on the code, i use the u2uxd. Dont forget to invert the rx/tx with the sensor ;)
*/

#include <HardwareSerial.h>
#include <TinyGPS++.h>

#define RXD2 16
#define TXD2 17

char fecha[20] = " ";
float latitud = 0, longitud = 0;
unsigned long previousMillis = 0;        // will store last time LED was updated
const long intervalo = 5000;
TinyGPSPlus gps;

void setup() {
  Serial.begin(115200);
  Serial1.begin(9600, SERIAL_8N1, RXD2, TXD2); //gps baud
}

String GPS(){
  
  if (gps.location.isUpdated() && gps.altitude.isUpdated())
  {
    Serial.print("D/M/A: ");
    Serial.print(gps.date.value());
    Serial.print(" | alt: ");
    Serial.print(gps.altitude.feet());
    Serial.print(" | satellites: ");
    Serial.println(gps.satellites.value());
    Serial.print(" | presición: ");
    Serial.println(gps.hdop.value());
    Serial.print(" | Altitud: ");
    Serial.println(gps.altitude.meters());
    Serial.print(" | Latitud: ");
    latitud = gps.location.lat();
      longitud = gps.location.lng();
    Serial.println(String(latitud,6));
    Serial.print(" | Longitud: ");
    Serial.println(String(longitud,6));
    sprintf(fecha, "%02d-%02d-%02dT%02d:%02d:%02d", gps.date.year(), gps.date.month(),
            gps.date.day(), gps.time.hour(), gps.time.minute(), gps.time.second());
    Serial.print(" | Fecha: ");
    Serial.println(fecha);
    return String(fecha) + "*" + String(latitud,6) + "*" + String(longitud,6);
  }
}

void loop() {
  bool recebido = false;
  if (Serial1.available()) {
    char cIn = Serial1.read();
    recebido = gps.encode(cIn);
  }
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= intervalo) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    //Fecha*latitud*longitud
    Serial.println(GPS());
  }
  
  
}
