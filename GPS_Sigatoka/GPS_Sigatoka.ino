/*
  SISTEMA PARA IDENTIFICAR ESTADOS DE LA SIGATOKA
  EN BANANO EN GPS A PARTIR DEL ALMACENAMIENTO EN UN SD
  Y EXTRACCIÓN POR MEDIO DE UN SERVIDOR WEB
  CARGADO EN LA ESP CON ARCHIVOS EN SPIFFS

  DESARROLLADO POR EL GURPO DE INVESTIGACIÓN
  GIDEAM
  UNIVERSIDAD DEL MAGDALENA
*/

/* Importando Librerías */

// Librería para generar el Access Point
#include <WiFi.h>
// Librería para crear el Sevidor Web
#include "ESPAsyncWebServer.h"
// Librería para el manejar archivos dentro de la ESP
// Serial Peripheral Interface Flash File System (SPIFFS)
// Flash de interfaz periférica serie (SPIFFS)
#include "SPIFFS.h"
// Librería para el control de la SD
#include <SD.h>
// Librería para utilizar el protocolo SPI
#include "SPI.h"
// Librería para manejar archivos en la SD o SPIFFS
#include "FS.h"
/* Librerías para controlar GPS*/
// Librería para el control del otro puerto serial
#include <HardwareSerial.h>
// Librería del GPS
#include <TinyGPS++.h>

/* Variables para la red AP */

// Variables de la red AP
const char *ssid = "GIDEAMSERVER";
const char *password = "1234567890";

// Crea el servidor web en el puerto 80
AsyncWebServer server(80);

/* Variables para el control de la SD */

// Ruta y nombre del archivo donde se almacena la información
#define PATH ("/prueba1.json")
#define PATHSD ("/datos_GPS.txt")
#define pinSD 5

/* Variables para control de sensores */

// Led de control
const int ledPin = 2;
// Led de SD
const int ledSD = 4;
// Led de GPS
const int ledGPS = 15;
// Botón de control
boolean botonPin = false;
// Variable con la data
String dataMessage;

// Variables de control para el GPS
// Pines GPIO a usar de Rx y Tx
static const int RX2 = 16, TX2 = 17;

// Variable con la fecha extraida del GPS
char fecha[20] = " ";
// Variables con la latitud y longitud extraida del GPS
float latitud = 0, longitud = 0, altitudG = 0, precision = 0;
// Variable para el control del GPS
TinyGPSPlus gps;
// Variable de control de los baudios de comunicación GPS
static const uint32_t GPSBaud = 9600;


/* Variables para el control del tiempo de muestreo */
unsigned long previousMillis = 0;
const long intervalo = 5000;
int counter = 0;

// Variable que controla la cantidad de muestras
int muestras = 0;

// Variable que controla el tipo de Sigatoka
int tipo = 0;

/* ---------------- FUNCIONES DE CONTROL ---------------- */

/* Funciones para manejo de archivos y directorios */

// Función para leer el archivo en la SD y enviarlo al servidor
String leerArchivo(fs::FS &fs, const char *path) {
  String payload;
  Serial.printf("Leyendo el archivo: %s\n", path);

  File file = fs.open(path);
  if (file) {
    while (file.available()) {
      char ch = file.read();
      payload += String(ch);
    }
    file.close();
    int tamanio = payload.length();
    payload[tamanio - 1] = ']';
    payload += '}';
    Serial.println("enviados--------------------------");
    return payload;
  } else {
    Serial.println("Falla al abrir el archivo para lectura");
    return "{\"message\":\"error\"}";
  }
}

// Función para leer un archivo en la SD
void readFile(fs::FS &fs, const char *path) {
  Serial.printf("Leyendo el archivo: %s\n", path);

  File file = fs.open(path);
  if (!file) {
    Serial.println("Falla al abrir el archivo para lectura");
    return;
  }

  Serial.print("Lectura del archivo: ");
  while (file.available()) {
    Serial.write(file.read());
  }
  file.close();
}

// Función para escribir un archivo en la SD
void writeFile(fs::FS &fs, const char *path, const String mensaje) {
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

// Función para anexar contenido a un archivo en la SD
void appendFile(fs::FS &fs, const char *path, const char *message) {
  Serial.printf("Añadiendo al archivo: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file) {
    Serial.println("Fallo al abrir el archivo para anexar información");
    return;
  }
  if (file.print(message)) {
    Serial.println("Mensaje añadido");
  } else {
    Serial.println("Fallo al añadir mensaje");
  }
  file.close();
}

// Función para renombrar un archivo en la SD
void renameFile(fs::FS &fs, const char *path1, const char *path2) {
  Serial.printf("Renombrando el archivo %s a %s\n", path1, path2);
  if (fs.rename(path1, path2)) {
    Serial.println("Archivo renonmbrado");
  } else {
    Serial.println("Fallo al renombrar el archivo");
  }
}


/* Funciones para el control del sistema */

// Función para iniciar la SD
void initSDCard() {
  if (!SD.begin(pinSD)) {
    Serial.println("Fallo en el montaje de la SD");
    digitalWrite(ledSD, HIGH);
    return;
  }
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE) {
    Serial.println("No hay tarjeta SD montada");
    digitalWrite(ledSD, HIGH);
    return;
  }
  Serial.print("Tipo de SD: ");
  if (cardType == CARD_MMC) {
    Serial.println("MMC");
  } else if (cardType == CARD_SD) {
    Serial.println("SDSC");
  } else if (cardType == CARD_SDHC) {
    Serial.println("SDHC");
  } else {
    Serial.println("UNKNOWN");
  }
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("Tamaño SD : %lluMB\n", cardSize);
}

// Función para iniciar el AP
void initAP() {
  // Creamos el punto de acceso
  WiFi.softAP(ssid, password);
  // Obtiene la IP del Punto de Acceso
  // Regularmente es 192.168.4.1
  IPAddress ip_ap = WiFi.softAPIP();
  // Imprime la IP
  Serial.println(ip_ap);
  Serial.print("Nombre de la red: ");
  Serial.println(ssid);
  Serial.print("Contraseña de la red: ");
  Serial.println(password);
}

/* --------------------- FUNCIONES DE SENSORES --------------------- */

float lat2 = 0;
// Función global para obtener diversos datos
String getSensorsData(String tipo) {
  if (botonPin) {

    String object_sensors = "{";
    object_sensors.concat("\"Muestra\":");
    object_sensors.concat(muestras);
    object_sensors.concat(",\"Tipo\":");
    object_sensors.concat(String(tipo));
    object_sensors.concat(",\"Fecha\":");
    object_sensors.concat("\"" + String(fecha) + "\"");
    object_sensors.concat(",\"Latitud\":");
    object_sensors.concat(String(latitud, 6));
    object_sensors.concat(",\"Longitud\":");
    object_sensors.concat(String(longitud, 6));
    object_sensors.concat(",\"Presición GPS\":");
    object_sensors.concat(precision);
    object_sensors.concat(",\"Altitud GPS\":");
    object_sensors.concat(altitudG);
    object_sensors.concat("}");

    Serial.print(object_sensors);

    dataMessage = String(muestras) + "," + String(tipo) + "," + String(fecha) + "," + String(latitud, 6) + "," + String(longitud, 6) + "," + String(precision) + "," + String(altitudG) + "\r\n";
    Serial.print("Salvando la data: ");
    Serial.println(dataMessage);

    // Añade al archivo txt
    appendFile(SD, PATHSD, dataMessage.c_str());

    return object_sensors;
  }
}

String controlSistema() {
  botonPin = !botonPin;
  if (botonPin) {
    digitalWrite(ledPin, HIGH);
    String(muestras++);
    return ("Funcionando");
  } else {
    digitalWrite(ledPin, LOW);
    return ("Apagado");
  }
}

String GPS() {

  if (gps.location.isUpdated() && gps.altitude.isUpdated()) {
    latitud = gps.location.lat();
    longitud = gps.location.lng();
    precision = gps.hdop.value();
    altitudG = gps.altitude.meters();
    sprintf(fecha, "%02d-%02d-%02dT%02d:%02d:%02d", gps.date.year(), gps.date.month(),
            gps.date.day(), gps.time.hour(), gps.time.minute(), gps.time.second());

    Serial.print(" | satellites: ");
    Serial.println(gps.satellites.value());
    Serial.print(" | presición: ");
    Serial.println(precision);
    Serial.print(" | Altitud: ");
    Serial.println(altitudG);
    Serial.print(" | Latitud: ");
    Serial.println(String(latitud, 6));
    Serial.print(" | Longitud: ");
    Serial.println(String(longitud, 6));
    if (latitud == 0) {
      digitalWrite(ledGPS, HIGH);
    } else {
      digitalWrite(ledGPS, LOW);
    }
    return String(fecha) + "*" + String(latitud, 6) + "*" + String(longitud, 6);
  } else {
    digitalWrite(ledGPS, HIGH);
  }
}

void setup() {
  /* Funciones de inicio del sistema */

  // Se inicia el puerto serial
  Serial.begin(115200);

  // Espera hasta que el puerto serial se conecte
  while (!Serial) {
    Serial.print(".");
  }

  // Se inicia el AP
  initAP();

  // Habilita como salida el pin del led que muestra si los datos están capturándose y los apaga
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  // Habilita como salida el pin del led que muestra si la SD falla y los apaga
  pinMode(ledSD, OUTPUT);
  digitalWrite(ledSD, LOW);

  // Habilita como salida el pin del led que muestra si el GPS falla y los apaga
  pinMode(ledGPS, OUTPUT);
  digitalWrite(ledGPS, LOW);

  /* SERVICIOS WEB  */

  // Cuando ingresas a la ruta root / de la página web
  // Muestra lo que se hay en el archivo index.html
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(SPIFFS, "/index.html", "text/html");
  });

  server.on("/estado1", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", getSensorsData("1").c_str());
  });
  server.on("/estado2", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", getSensorsData("2").c_str());
  });
  server.on("/estado3", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", getSensorsData("3").c_str());
  });
  server.on("/estado4", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", getSensorsData("4").c_str());
  });
  server.on("/estado5", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", getSensorsData("5").c_str());
  });
  server.on("/estado6", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", getSensorsData("6").c_str());
  });


  // Servicio de control del sistema
  // Enciende o apaga el sistema
  server.on("/sistema", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send_P(200, "text/plain", controlSistema().c_str());
  });

  // Inicia el servidor
  server.begin();
  Serial.println("Servidor HTTP iniciado");

  /* SERVICIOS DE ADMINISTRACIÓN DE DATOS
      Memoria SD
      Sistema de archivos SPIFFS
  */

  // Se inicia la SD
  initSDCard();

  // If the data.txt file doesn't exist
  // Create a file on the SD card and write the data labels
  File file = SD.open(PATHSD);
  if (!file) {
    Serial.println("El Archivo no existe");
    Serial.println("Creando el archivo...");
    writeFile(SD, PATHSD, "Muestra, Tipo, Fecha, Latitud, Longitud, Presición GPS, Altitud GPS \r\n");
  } else {
    Serial.println("El archivo existe todo OK");
    digitalWrite(ledSD, LOW);
  }
  file.close();

  // Inicializando SPIFFS
  if (!SPIFFS.begin(true)) {
    Serial.println("Sistema de archivos SPIFFS fallida");
    return;
  }
  Serial.println("Inicialización SPIFFS lista.");

  /*
        GPS
  */

  // Inicia el sensor GPS
  Serial1.begin(GPSBaud, SERIAL_8N1, RX2, TX2);
}

void loop() {
  /*
    Funciones que se estarán ejecutando cada cierto tiempo
    Permitiendo que se ejecute un sistema de monitoreo en tiempo real
    A partir del tiempo designado.
    Permitiendo que en paralelo se adquieran los datos de los sensores
    cada cierto tiempo y cuando se ingrese a conocer el estado actual
    Se visualicen los datos más recientes.
  */

  /*
  */

  bool recibido = false;
  if (Serial1.available()) {
    // Serial.println("Entré");
    char cIn = Serial1.read();
    recibido = gps.encode(cIn);
  }
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= intervalo) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    // Fecha*latitud*longitud
    Serial.println(GPS());
  }
  if (lat2 == latitud) {
    digitalWrite(ledGPS, HIGH);
  } else {
    digitalWrite(ledGPS, LOW);
  }
  lat2 == latitud;

}
