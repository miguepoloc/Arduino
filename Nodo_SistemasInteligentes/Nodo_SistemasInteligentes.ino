

// Código para el nodo de Sistemas Inteligentes

// Librería para el sensor de temperatura del suelo
// #include <Adafruit_MAX31865.h>

// Se incluyen las librerías de la SD
#include <SPI.h>
#include <SD.h>
#include <FS.h>


#include <WebServer.h>
#include <Adafruit_BME280.h>

#define SEALEVELPRESSURE_HPA (1013.25)

Adafruit_BME280 bme;

float temperature, humidity, pressure, altitude;

/*Put your SSID & Password*/
//const char* ssid = "LAB PROCESOS";  // Enter SSID here
//const char* password = "Pr0c3s0sUn1m4gd4l3n@.2017#*";  //Enter Password here

//WebServer server(80);

void setup() {
  Serial.begin(115200);
  delay(100);

  bme.begin(0x76);
  

  //server.on("/hola/", handle_OnConnect);
  //server.onNotFound(handle_NotFound);

  //server.begin();
  //Serial.println("HTTP server started");
  Serial.print("Inicializando SD card...");

  if (!SD.begin(5)) {
    Serial.println("Inicialización fallida!");
    while (1);
  }
  Serial.println("Inicialización lista.");
 
  //writeFile(SD, "/hello2.txt", "{\"temperatura\": 12, \"humedad\":14}");
  
}



void writeFile(fs::FS &fs, const char * path,const String mensaje) {
  Serial.printf("Escribiendo el archivo: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Error al abrir el archivo para escribirlo");
    return;
  }
  if (file.print(mensaje)) {
    Serial.println("Archivo escrito");
  } else {
    Serial.println("Error al escribir");
  }
  file.close();
}

void readFile(fs::FS &fs, const char * path){
  Serial.printf("Reading file: %s\n", path);

  File file = fs.open(path);
  if(!file){
    Serial.println("Failed to open file for reading");
    return;
  }

  Serial.print("Read from file: ");
  while(file.available()){
    Serial.write(file.read());
  }
  file.close();
}

void appendFile(fs::FS &fs, const char * path, const char * message){
  Serial.printf("Appending to file: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if(!file){
    Serial.println("Failed to open file for appending");
    writeFile(SD, path, "");
  }
  if(file.print(message)){
      Serial.println("Message appended");
  } else {
    Serial.println("Append failed");
  }
  file.close();
}



char Buf[50];
void loop() {
  String msg = "{\"temperatura\": ";
  temperature = bme.readTemperature();
  humidity = bme.readHumidity();
  pressure = bme.readPressure() / 100.0F;
  altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
  Serial.print("Prueba = "); 
  Serial.println(temperature);
  Serial.print("Humedad = "); 
  Serial.println(humidity);
  
  msg.concat(String(temperature));
  msg.concat(", \"humedad\":");
  msg.concat(String(humidity));
  msg.concat("}");
  msg.toCharArray(Buf, 50);
  appendFile(SD, "/prueba3.json", "buf");
  delay(10000);
  
  readFile(SD, "/prueba3.json");
  
}
