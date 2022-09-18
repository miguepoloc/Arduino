// Librerías a utilizar
#include <TinyGPS++.h>
#include <SoftwareSerial.h>

static const int RXPin = 3, TXPin = 1;
//static const int RXPin = 16, TXPin = 17;
static const uint32_t GPSBaud = 9600;
int ledPin = 2;

// The TinyGPS++ object
TinyGPSPlus gps;

// The serial connection to the GPS device
SoftwareSerial ss(RXPin, TXPin);

char fecha[20] = "";
float latitud = 0, longitud = 0;
unsigned long previousMillis = 0;        // will store last time LED was updated
const long intervalo = 5000;

void setup() {
  Serial.begin(115200);
  ss.begin(GPSBaud);
  pinMode (ledPin, OUTPUT);
}

void loop() {
  // This sketch displays information every time a new sentence is correctly encoded.
  GPS();

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= intervalo) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    Serial.print("Fecha= ");
    Serial.print(fecha);
    Serial.print(" Latitud= ");
    Serial.print(latitud, 6);
    Serial.print(" Longitud= ");
    Serial.println(longitud, 6);
  }
}

void GPS() {
  while (ss.available() > 0) {
    gps.encode(ss.read());
    if (gps.location.isUpdated()) {
      digitalWrite (ledPin, HIGH);
      //Fecha y hora
      sprintf(fecha, "%02d-%02d-%02dT%02d:%02d:%02d", gps.date.year(), gps.date.month(),
              gps.date.day(), gps.time.hour(), gps.time.minute(), gps.time.second());
      //Longitud y latitud
      latitud = gps.location.lat();
      longitud = gps.location.lng();
    }
    else {
      digitalWrite (ledPin, LOW);
    }
  }
}
