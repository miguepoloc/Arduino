/*
  SISTEMA DE MONITOREO DE VARIABLES AMBIENTALES
  A PARTIR DEL ALMACENAMIENTO EN UN SD
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
// Librería que permite la comunicación I2C
#include <Wire.h>
/* Librerías para controlar sensores*/
// Librería para controlar el sensor SHT21
#include "SHT21.h"
// Librería del sensor BME280
#include <Adafruit_BME280.h>
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
#define PATHSD ("/tiemporeal1.txt")
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

// Variables para almacenar datos de los sensores
String temperaturaA, humedadA, presionA, altitud, PH, EC, RTD, tempsht, humsht, humS;

// varibles pin analógico humedad del suelo
const int portPin = 34;
int valorAnalogico = 0;

// Variable de control del Sensor de temperatura y humedad SHT21
SHT21 SHT21;

// Variables de control para el sensor de temperatura, humedad y presión BME280
#define SEALEVELPRESSURE_HPA (1013.25)
#define BME_ADDRESS (0x76)
Adafruit_BME280 bme;

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

// Variables para el sensor PH
#define addressPH 99 // 0x63      //default I2C ID number for EZO pH Circuit.
byte codePH = 0;     // used to hold the I2C response code.
char ph_data[20];    // we make a 20 byte character array to hold incoming data from the RTD circuit.
byte in_charPH = 0;  // used as a 1 byte buffer to store in bound bytes from the RTD Circuit.
byte iPH = 0;        // counter used for ph_data array.
int time_PH = 900;   // used to change the delay needed depending on the command sent to the EZO Class RTD Circuit.
float ph_float;      // float var used to hold the float value of the RTD.

// Variables para el sensor EC conductividad
#define addressEC 0x64 // default I2C ID number for EZO EC Circuit.
byte codeEC = 0;       // used to hold the I2C response code.
char ec_data[20];      // we make a 20 byte character array to hold incoming data from the RTD circuit.
byte in_charEC = 0;    // used as a 1 byte buffer to store in bound bytes from the RTD Circuit.
byte iEC = 0;          // counter used for ec_data array.
int time_EC = 600;     // used to change the delay needed depending on the command sent to the EZO Class RTD Circuit.600
float ec_float;        // float var used to hold the float value of the RTD

// Variables para el sensor de temperatura RTD
#define addressRTD 0x66
byte codeRTD = 0;
char rtd_data[20];
byte in_charRTD = 0;
byte iRTD = 0;
int time_RTD = 600;
float rtd_float;

/* Variables para el control del tiempo de muestreo */

unsigned long previousMillis = 0;
const long intervalo = 10000;
int counter = 0;

// Variable que controla la cantidad de muestras
int muestras = 0;

/* ---------------- FUNCIONES DE CONTROL ---------------- */

/* Funciones para manejo de archivos y directorios */

// Función para leer el archivo en la SD y enviarlo al servidor
String leerArchivo(fs::FS &fs, const char *path)
{
  String payload;
  Serial.printf("Leyendo el archivo: %s\n", path);

  File file = fs.open(path);
  if (file)
  {
    while (file.available())
    {
      char ch = file.read();
      payload += String(ch);
    }
    file.close();
    int tamanio = payload.length();
    payload[tamanio - 1] = ']';
    payload += '}';
    Serial.println("enviados--------------------------");
    return payload;
  }
  else
  {
    Serial.println("Falla al abrir el archivo para lectura");
    return "{\"message\":\"error\"}";
  }
}

// Función para leer un archivo en la SD
void readFile(fs::FS &fs, const char *path)
{
  Serial.printf("Leyendo el archivo: %s\n", path);

  File file = fs.open(path);
  if (!file)
  {
    Serial.println("Falla al abrir el archivo para lectura");
    return;
  }

  Serial.print("Lectura del archivo: ");
  while (file.available())
  {
    Serial.write(file.read());
  }
  file.close();
}

boolean checkChar(fs::FS &fs, const char *path, const char ch)
{
  File file = fs.open(path);

  if (!file)
  {
    Serial.println("Failed to open file for reading");
    return false;
  }

  Serial.print("Read from file for check char: ");
  while (file.available())
  {
    char mychar = file.read();
    if (mychar == ch)
    {
      file.close();
      return true;
    }
  }
  file.close();
  return false;
}

boolean checkFile(fs::FS &fs, const char *path)
{

  File file = fs.open(path);
  if (!file)
  {
    Serial.println("Failed to open file for reading");
    file.close();
    return false;
  }

  Serial.printf("Verificando archivo: %s\n", path);
  if (file.available())
  {
    file.close();
    return true;
  }
}

// Función para borrar un archivo en la SD
void deleteFile(fs::FS &fs, const char *path)
{
  Serial.printf("Borrando el archivo: %s\n", path);
  if (fs.remove(path))
  {
    Serial.println("Archivo borrado");
  }
  else
  {
    Serial.println("Falla en borrar el archivo");
  }
}

// Función para escribir un archivo en la SD
void writeFile(fs::FS &fs, const char *path, const String mensaje)
{
  Serial.printf("Escribiendo el archivo: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if (!file)
  {
    Serial.println("Error al abrir el archivo para escribirlo");
    return;
  }
  if (file.print(mensaje))
  {
    Serial.println("Archivo escrito");
  }
  else
  {
    Serial.println("Error al escribir");
  }
  file.close();
}

// Función para anexar contenido a un archivo en la SD
void appendFile(fs::FS &fs, const char *path, const char *message)
{
  Serial.printf("Añadiendo al archivo: %s\n", path);

  File file = fs.open(path, FILE_APPEND);
  if (!file)
  {
    Serial.println("Fallo al abrir el archivo para anexar información");
    return;
  }
  if (file.print(message))
  {
    Serial.println("Mensaje añadido");
  }
  else
  {
    Serial.println("Fallo al añadir mensaje");
  }
  file.close();
}

// Función para renombrar un archivo en la SD
void renameFile(fs::FS &fs, const char *path1, const char *path2)
{
  Serial.printf("Renombrando el archivo %s a %s\n", path1, path2);
  if (fs.rename(path1, path2))
  {
    Serial.println("Archivo renonmbrado");
  }
  else
  {
    Serial.println("Fallo al renombrar el archivo");
  }
}

// Función para testear un archivo en la SD
// Muestra cuánto tiempo se tarda en leer el contenido de un archivo
void testFileIO(fs::FS &fs, const char *path)
{
  File file = fs.open(path);
  static uint8_t buf[512];
  size_t len = 0;
  uint32_t start = millis();
  uint32_t end = start;
  if (file)
  {
    len = file.size();
    size_t flen = len;
    start = millis();
    while (len)
    {
      size_t toRead = len;
      if (toRead > 512)
      {
        toRead = 512;
      }
      file.read(buf, toRead);
      len -= toRead;
    }
    end = millis() - start;
    Serial.printf("%u bytes leídos en %u ms\n", flen, end);
    file.close();
  }
  else
  {
    Serial.println("Falla al abrir el archivo para lectura");
  }

  file = fs.open(path, FILE_WRITE);
  if (!file)
  {
    Serial.println("Falla en abrir el archivo para la escritura");
    return;
  }

  size_t i;
  start = millis();
  for (i = 0; i < 2048; i++)
  {
    file.write(buf, 512);
  }
  end = millis() - start;
  Serial.printf("%u bytes escritos en %u ms\n", 2048 * 512, end);
  file.close();
}

/* Funciones para manejo de directorios */

// Función que enlista los directorios de la tarjeta SD.
// Esta función acepta como argumentos el sistema de archivos (Sd)
// El nombre del directorio principal y los niveles que se van a introducir en el directorio.
void listDir(fs::FS &fs, const char *dirname, uint8_t levels)
{
  Serial.printf("Listando el directorio: %s\r\n", dirname);

  File root = fs.open(dirname);
  if (!root)
  {
    Serial.println("- Fallo al abrir el directorio");
    return;
  }
  if (!root.isDirectory())
  {
    Serial.println(" - No es un directorio");
    return;
  }

  File file = root.openNextFile();
  while (file)
  {
    if (file.isDirectory())
    {
      Serial.print("  DIRECTORIO: ");
      Serial.println(file.name());
      if (levels)
      {
        listDir(fs, file.path(), levels - 1);
      }
    }
    else
    {
      Serial.print("  ARCHIVO: ");
      Serial.print(file.name());
      Serial.print("\tTAMAÑO: ");
      Serial.println(file.size());
    }
    file = root.openNextFile();
  }
}

// Función para crear un directorio en la SD
void createDir(fs::FS &fs, const char *path)
{
  Serial.printf("Creando el Directorio: %s\n", path);
  if (fs.mkdir(path))
  {
    Serial.println("Directorio creado");
  }
  else
  {
    Serial.println("Creación de directorio fallida");
  }
}

// Función para borrar un directorio en la SD
void removeDir(fs::FS &fs, const char *path)
{
  Serial.printf("Eliminando el directorio: %s\n", path);
  if (fs.rmdir(path))
  {
    Serial.println("Directorio borrado");
  }
  else
  {
    Serial.println("Borrado de directorio fallido");
  }
}

/* Funciones para el control del sistema */

// Función para iniciar la SD
void initSDCard()
{
  if (!SD.begin(pinSD))
  {
    Serial.println("Fallo en el montaje de la SD");
    digitalWrite(ledSD, HIGH);
    return;
  }
  uint8_t cardType = SD.cardType();

  if (cardType == CARD_NONE)
  {
    Serial.println("No hay tarjeta SD montada");
    digitalWrite(ledSD, HIGH);
    return;
  }
  Serial.print("Tipo de SD: ");
  if (cardType == CARD_MMC)
  {
    Serial.println("MMC");
  }
  else if (cardType == CARD_SD)
  {
    Serial.println("SDSC");
  }
  else if (cardType == CARD_SDHC)
  {
    Serial.println("SDHC");
  }
  else
  {
    Serial.println("UNKNOWN");
  }
  uint64_t cardSize = SD.cardSize() / (1024 * 1024);
  Serial.printf("Tamaño SD : %lluMB\n", cardSize);
}

// Función para iniciar el AP
void initAP()
{
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

// Temperatura del sensor SHT21
String readSHT21Temperatura()
{
  float temp = SHT21.getTemperature();
  if (isnan(temp))
  {
    Serial.println("Error al leer la temperatura del SHT21");
    return "--";
  }
  else
  {
    Serial.println("Leí la temperatura y fue: ");
    Serial.println(temp);
    return String(temp);
  }
}

// Humedad del sensor SHT21
String readSHT21Humedad()
{
  float hum = SHT21.getHumidity();
  if (isnan(hum))
  {
    Serial.println("Error al leer la humedad del SHT21");
    return "--";
  }
  else
  {
    Serial.println(hum);
    return String(hum);
  }
}

// Temperatura del sensor BME280
String readBME280Temperature()
{
  // Lee la temperatura por defecto en Celsius
  float t = bme.readTemperature();
  if (isnan(t))
  {
    Serial.println("Error al leer la temperatura del sensor BME280");
    return "--";
  }
  else
  {
    Serial.println(t);
    return String(t);
  }
}

// Humedad relativa del sensor BME280
String readBME280Humidity()
{
  float h = bme.readHumidity();
  if (isnan(h))
  {
    Serial.println("Error al leer la humedad del sensor BME280");
    return "--";
  }
  else
  {
    Serial.println(h);
    return String(h);
  }
}

// Presión atmosférica del sensor BME280
String readBME280Pressure()
{
  float p = bme.readPressure() / 100.0F;
  if (isnan(p))
  {
    Serial.println("Error al leer la presión del sensor BME280");
    return "--";
  }
  else
  {
    Serial.println(p);
    return String(p);
  }
}

// Altura sobre el nivel del mar del sensor BME280
String readBME280Altitude()
{
  float a = bme.readAltitude(SEALEVELPRESSURE_HPA);

  if (isnan(a))
  {
    Serial.println("Error al leer la altura del sensor BME280");
    return "--";
  }
  else
  {
    Serial.println(a);
    return String(a);
  }
}

// Sensor de PH de Atlas Scientific
String sensorPH()
{
  boolean flag = true;

  Wire.beginTransmission(addressPH); // call the circuit by its ID number.
  Wire.write("R \n");                // transmit the command that was sent through the serial port.
  Wire.endTransmission();            // end the I2C data transmission.

  while (flag)
  {

    delay(time_PH);                     // wait the correct amount of time for the circuit to complete its instruction.
    Wire.requestFrom(addressPH, 20, 1); // call the circuit and request 20 bytes (this may be more than we need)
    codePH = Wire.read();

    switch (codePH)
    { // switch case based on what the response code is.
      case 1:                        // decimal 1.
        Serial.println("Success"); // means the command was successful.
        while (Wire.available())
        { // are there bytes to receive.
          in_charPH = Wire.read();  // receive a byte.
          ph_data[iPH] = in_charPH; // load this byte into our array.
          iPH += 1;                 // incur the counter for the array element.
          if (in_charPH == 0)
          { // if we see that we have been sent a null command.
            iPH = 0; // reset the counter i to 0.
            break;   // exit the while loop.
          }
        }

        // Serial.println("pH: " + String(ph_data));             //print the data.

        return String(ph_data);

      case 2:                       // decimal 2.
        Serial.println("Failed"); // means the command has failed.
        return String("0.000");

      case 254:                      // decimal 254.
        Serial.println("Pending"); // means the command has not yet been finished calculating.
        break;                     // exits the switch case.

      case 255:                      // decimal 255.
        Serial.println("No Data"); // means there is no further data to send.
        return String("0.000");
    }
  }
}

// Sensor de Conductividad de Atlas Scientific
String sensorEC()
{
  boolean flag = true;

  Wire.beginTransmission(addressEC); // call the circuit by its ID number.
  Wire.write("R \n");                // transmit the command that was sent through the serial port.
  Wire.endTransmission();            // end the I2C data transmission.

  while (flag)
  {

    delay(time_EC); // wait the correct amount of time for the circuit to complete its instruction.

    Wire.requestFrom(addressEC, 20, 1); // call the circuit and request 20 bytes (this may be more than we need)
    codeEC = Wire.read();               // the first byte is the response code, we read this separately.

    switch (codeEC)
    { // switch case based on what the response code is.
      case 1:                        // decimal 1.
        Serial.println("Success"); // means the command was successful.
        while (Wire.available())
        { // are there bytes to receive.
          in_charEC = Wire.read();  // receive a byte.
          ec_data[iEC] = in_charEC; // load this byte into our array.
          iEC += 1;                 // incur the counter for the array element.
          if (in_charEC == 0)
          { // if we see that we have been sent a null command.
            iEC = 0; // reset the counter i to 0.
            break;   // exit the while loop.
          }
        }
        return String(ec_data);
      case 2:                       // decimal 2.
        Serial.println("Failed"); // means the command has failed.
        return String("0.000");   // exits the switch case.

      case 254:                      // decimal 254.
        Serial.println("Pending"); // means the command has not yet been finished calculating.
        break;                     // exits the switch case.

      case 255:                      // decimal 255.
        Serial.println("No Data"); // means there is no further data to send.
        return String("0.000");
    }

    // Serial.println("EC: " + String(ec_data));             //print the data.
  }
}

// Sensor de Temperatura del agua o suelo de Atlas Scientific
String sensorRTD()
{
  boolean flag = true;

  Wire.beginTransmission(addressRTD); // call the circuit by its ID number.
  Wire.write("R \n");                 // transmit the command that was sent through the serial port.
  Wire.endTransmission();             // end the I2C data transmission.

  while (flag)
  {

    delay(time_RTD); // wait the correct amount of time for the circuit to complete its instruction.

    Wire.requestFrom(addressRTD, 20, 1); // call the circuit and request 20 bytes (this may be more than we need)
    codeRTD = Wire.read();               // the first byte is the response code, we read this separately.

    switch (codeRTD)
    { // switch case based on what the response code is.
      case 1:                        // decimal 1.
        Serial.println("Success"); // means the command was successful.
        while (Wire.available())
        { // are there bytes to receive.
          in_charRTD = Wire.read();    // receive a byte.
          rtd_data[iRTD] = in_charRTD; // load this byte into our array.
          iRTD += 1;                   // incur the counter for the array element.
          if (in_charRTD == 0)
          { // if we see that we have been sent a null command.
            iRTD = 0; // reset the counter i to 0.
            break;    // exit the while loop.
          }
        }
        return String(rtd_data);
      case 2:                       // decimal 2.
        Serial.println("Failed"); // means the command has failed.
        return String("0.000");   // exits the switch case.

      case 254:                      // decimal 254.
        Serial.println("Pending"); // means the command has not yet been finished calculating.
        break;                     // exits the switch case.

      case 255:                      // decimal 255.
        Serial.println("No Data"); // means there is no further data to send.
        return String("0.000");
    }

    // Serial.println("EC: " + String(ec_data));             //print the data.
  }
}

// Función para extraer datos analógicos de la humedad del suelo
String sueloHumedad()
{
  valorAnalogico = analogRead(portPin);
  return String(valorAnalogico);
}

// Función global para obtener diversos datos
String getSensorsData()
{
  if (botonPin)
  {
    tempsht = readSHT21Temperatura();
    humsht = readSHT21Humedad();
    temperaturaA = readBME280Temperature();
    humedadA = readBME280Humidity();
    presionA = readBME280Pressure();
    altitud = readBME280Altitude();
    RTD = sensorRTD();
    humS = sueloHumedad();

    String object_sensors = "{";
    object_sensors.concat("\"Muestra\":");
    object_sensors.concat(muestras);
    object_sensors.concat(",\"Temperatura SHT\":");
    object_sensors.concat(tempsht);
    object_sensors.concat(",\"Humedad SHT\":");
    object_sensors.concat(humsht);
    object_sensors.concat(",\"Temperatura BME\":");
    object_sensors.concat(temperaturaA);
    object_sensors.concat(",\"Humedad BME\":");
    object_sensors.concat(humedadA);
    object_sensors.concat(",\"Presión BME\":");
    object_sensors.concat(presionA);
    object_sensors.concat(",\"Altitud BME\":");
    object_sensors.concat(altitud);
    object_sensors.concat(",\"Temperatura Suelo\":");
    object_sensors.concat(RTD);
    object_sensors.concat(",\"Humedad Suelo\":");
    object_sensors.concat(humS);
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

    dataMessage = String(muestras) + "," + String(tempsht) + "," + String(humsht) +
                  "," + String(temperaturaA) + "," + String(humedadA) + "," + String(presionA) + "," +
                  String(altitud) + "," + String(RTD) + "," + String(humS) + "," + String(latitud, 6) + "," +
                  String(longitud, 6) + "," + String(precision) + "," + String(altitudG) + "\r\n";
    Serial.print("Salvando la data: ");
    Serial.println(dataMessage);

    // Añade al archivo txt
    appendFile(SD, PATHSD, dataMessage.c_str());
    return object_sensors;
  }
}

String controlSistema()
{
  botonPin = !botonPin;
  if (botonPin)
  {
    digitalWrite(ledPin, HIGH);
    String(muestras++);
    return ("Funcionando");
  }
  else
  {
    digitalWrite(ledPin, LOW);
    return ("Apagado");
  }
}

String GPS()
{

  if (gps.location.isUpdated() && gps.altitude.isUpdated())
  {
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
    if (precision == 0) {
      digitalWrite(ledGPS, HIGH);
    }
    else {
      digitalWrite(ledGPS, LOW);
    }
    return String(fecha) + "*" + String(latitud, 6) + "*" + String(longitud, 6);
  }
}

void setup()
{
  /* Funciones de inicio del sistema */

  // Se inicia el puerto serial
  Serial.begin(115200);

  // Espera hasta que el puerto serial se conecte
  while (!Serial)
  {
    Serial.print(".");
  }

  // Se inicia el AP
  initAP();

  // Habilita el puerto I2C
  Wire.begin();

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
  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send(SPIFFS, "/index.html", "text/html");
  });

  // Cuando ingresas a la ruta /tiemporeal de la página web
  // Muestra lo que se hay en el archivo tiemporeal.html
  server.on("/tiemporeal", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send(SPIFFS, "/tiempo_real.html");
  });
  // Cuando se ingresa a tiemporeal.html se ejecuta una función en JavaScript
  // que hace una petición a /tempsht, lo que hace que se ejecute la función readSHT21Temperatura()
  // retornando el valor de temperatura y almacenándolo en una variable en JavaScript
  // para luego ser graficada allá.
  server.on("/tempsht", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send_P(200, "text/plain", readSHT21Temperatura().c_str());
  });
  // Solicitud realizada desde el JavaScript para la api /humedsht y ejecuta la función readSHT21Humedad()
  // Retornando el valor de humedad, almacenándolo en el JavaScript para luego ser graficado.
  server.on("/humedsht", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send_P(200, "text/plain", readSHT21Humedad().c_str());
  });

  server.on("/sensors", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send_P(200, "text/plain", getSensorsData().c_str());
  });

  // Servicio de control del sistema
  // Enciende o apaga el sistema
  server.on("/sistema", HTTP_GET, [](AsyncWebServerRequest * request)
  {
    request->send_P(200, "text/plain", controlSistema().c_str());
  });

  // Servicios para las gráficas
  //    server.on("/lib/highstock/highstock.js", HTTP_GET, [](AsyncWebServerRequest *request)
  //              { request->send(SPIFFS, "/lib/highstock/highstock.js"); });
  //    server.on("/lib/highstock/data.js", HTTP_GET, [](AsyncWebServerRequest *request)
  //              { request->send(SPIFFS, "/lib/highstock/data.js"); });
  //    server.on("/lib/highstock/accessibility.js", HTTP_GET, [](AsyncWebServerRequest *request)
  //              { request->send(SPIFFS, "/lib/highstock/accessibility.js"); });
  //    server.on("/lib/highstock/data.js.map", HTTP_GET, [](AsyncWebServerRequest *request)
  //              { request->send(SPIFFS, "/lib/highstock/data.js.map"); });

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
  if (!file)
  {
    Serial.println("El Archivo no existe");
    Serial.println("Creando el archivo...");
    writeFile(SD, PATHSD, "Muestra, Temperatura sht, Humedad sht, Temperatura BME, Humedad BME, Presión BME, Altitud BME, Temperatura Suelo, Humedad Suelo, Latitud, Longitud, Presición GPS, Altitud GPS \r\n");
  }
  else
  {
    Serial.println("El archivo existe");
    digitalWrite(ledSD, LOW);
  }
  file.close();

  // Inicializando SPIFFS
  if (!SPIFFS.begin(true))
  {
    Serial.println("Sistema de archivos SPIFFS fallida");
    return;
  }
  Serial.println("Inicialización SPIFFS lista.");

  /* SENSORES
        SHT21: Temperatura y Humedad
        BME280: Temperatura, humedad, presión y altitud
        GPS
  */

  // Inicia el sensor SHT21
  SHT21.begin();
  Serial.println("Inicialización SHT21 lista.");

  // Inicia el sensor BME
  if (!bme.begin(BME_ADDRESS))
  {
    Serial.println("No hay un módulo BME conectado");
  }
  else
  {
    Serial.println("BME conectado");
  }

  // Inicia el sensor GPS
  Serial1.begin(GPSBaud, SERIAL_8N1, RX2, TX2);
}

void loop()
{
  /*
    Funciones que se estarán ejecutando cada cierto tiempo
    Permitiendo que se ejecute un sistema de monitoreo en tiempo real
    A partir del tiempo designado.
    Permitiendo que en paralelo se adquieran los datos de los sensores
    cada cierto tiempo y cuando se ingrese a conocer el estado actual
    Se visualicen los datos más recientes.
  */

  bool recebido = false;
  if (Serial1.available())
  {
    char cIn = Serial1.read();
    recebido = gps.encode(cIn);
  }
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= intervalo)
  {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    // Fecha*latitud*longitud
    Serial.println(GPS());
  }
}
