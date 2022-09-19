#include <TinyGPS++.h>
#include <SoftwareSerial.h>

static const int RXPin = 3, TXPin = 1;
static const uint32_t GPSBaud = 9600;

// The TinyGPS++ object
TinyGPSPlus gps;

// The serial connection to the GPS device
SoftwareSerial ss(RXPin, TXPin);

char fecha[20] = " ";
float latitud = 0, longitud = 0;
unsigned long previousMillis = 0;        // will store last time LED was updated
const long intervalo = 5000;

void setup(){
  Serial.begin(115200);
  ss.begin(GPSBaud);
}

void loop(){
  // Función para obtener los datos del GPS
  GPS();
  
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= intervalo) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    //Fecha*latitud*longitud
    Serial.println(datosGPS());
  }
}

//Método que lee los datos provenientes del GPS y los guarda en las variables correspondientes
void GPS() {
  while (ss.available() > 0){
    gps.encode(ss.read());
    if (gps.location.isUpdated()){
      //Fecha y hora
      sprintf(fecha, "%02d-%02d-%02dT%02d:%02d:%02d", gps.date.year(), gps.date.month(),
              gps.date.day(), gps.time.hour(), gps.time.minute(), gps.time.second());
      //Longitud y latitud
      latitud = gps.location.lat();
      longitud = gps.location.lng();
    }
  }
}

//Método que devuelve la cadena de caracteres correspondiente a los datos de fecha*latitud*longitud
String datosGPS() {
  return String(fecha) + "*" + String(latitud,6) + "*" + String(longitud,6);
}
