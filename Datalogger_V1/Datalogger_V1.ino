
// Librerías necesarias para el sensor DHT11
#include "DHT.h"

// Librerias necesarias para el sensor BMP180
#include <SFE_BMP180.h>
#include <Wire.h>

// Librerias necesarias para el sensor DS18B20
#include <OneWire.h>

// Librerias necesarias para el reloj
#include "RTClib.h"

// Librerías necesarias para la SD
#include <SPI.h>
#include <SD.h>

// Configuración del sensor DHT
#define DHTPIN 4          // Pin donde está conectado
#define DHTTYPE DHT11     // Se elige el tipo de sensor, en este caso DHT11
DHT dht(DHTPIN, DHTTYPE); // Se inicializa el sensor

// Configuración para el sensor BMP180
SFE_BMP180 presion;     // Se crea un objeto de SFE_BMP180 llamado
#define ALTURA_MAR 20.0 // Altura al nivel del mar del punto donde se ubicará el sensor

// Configuración para el sensor DS18B20
OneWire ds(3); // Se va a conectar el sensor por el pin 3
// Es necesario colocar una resistencia de 4.7K entre el pin y VCC

// Configuración para el reloj
RTC_DS3231 rtc;
String daysOfTheWeek[7] = {"Domingo", "Lunes", "Martes", "Miercoles", "Jueves", "Viernes", "Sabado"};
String monthsNames[12] = {"Enero", "Febrero", "Marzo", "Abril", "Mayo", "Junio", "Julio", "Agosto", "Septiembre", "Octubre", "Noviembre", "Diciembre"};

// Configuración para la SD
const int pinSD = 53; // El pin por el cual leerá los datos la SD
File archivo;


int led = 2;

void setup()
{
  // Se inicia el puerto serial en 9600
  Serial.begin(9600);
  pinMode(led, OUTPUT);
  // Inicializando el sensor DHT
  Serial.println("DHT11 iniciado");
  dht.begin();
  digitalWrite(led, LOW);
  // Inicializando el sensor BMP180
  if (presion.begin()) {
    Serial.println("BMP180 iniciado");
    digitalWrite(led, LOW);
  }
  else
  {
    // Algo ocurrió mal, verificar
    Serial.println("BMP180 FALLÓ\n\n");
    digitalWrite(led, HIGH);
    // No hace más nada y para el sistema
  }

  // Inicializando el reloj
  if (!rtc.begin())
  {
    Serial.println("No se encontró el RTC");
    digitalWrite(led, HIGH);
  }


  // When time needs to be set on a new device, or after a power loss, the
  // following line sets the RTC to the date & time this sketch was compiled
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  // This line sets the RTC with an explicit date & time, for example to set
  // January 21, 2014 at 3am you would call:
  // rtc.adjust(DateTime(2014, 1, 21, 3, 0, 0));



  // Si se ha perdido la corriente, fijar fecha y hora
  if (rtc.lostPower())
  {
    // Fijar a fecha y hora de compilacion
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }

  // Inicializando la SD

  if (!SD.begin(pinSD))
  {
    Serial.println("SD falló o no está instalada");
    // No hace más nada y para el sistema
    digitalWrite(led, HIGH);
  }
  Serial.println("SD iniciada");
  digitalWrite(led, LOW);


}

void loop()
{

  /* ************** SENSOR DHT ************** */
  // Se lee la humedad con el sensor DHT
  float humedad_DHT = dht.readHumidity();
  // Se lee la temperatura con el sensor DHT
  float temperatura_DHT = dht.readTemperature();

  // Revisa si el sensor ha fallado
  if (isnan(humedad_DHT) || isnan(temperatura_DHT))
  {
    Serial.println(F("Error al leer el sensor DHT"));
    digitalWrite(led, HIGH);
  }
  digitalWrite(led, LOW);

  // Muestra por el puerto serial la temperatura y la humedad
  Serial.println();
  Serial.print("Humedad: ");
  Serial.print(humedad_DHT);
  Serial.print("%  Temperatura: ");
  Serial.print(temperatura_DHT);
  Serial.println("°C ");

  /* ************** SENSOR BMP180 ************** */

  // Variable que controla el estado del sensor
  char status;
  // Crea las variables donde se van a almacenar los valores de los sensores
  double temperatura_BMP180, presion_BMP180, p0, altura;

  // Se imprime la altura sobre el nivel del mar
  Serial.println();
  Serial.print("Altura sobre el nivel del mar: ");
  Serial.print(ALTURA_MAR, 0);
  Serial.println(" metros");

  // En estatus se almacena el tiempo en ms que se debe esperar para realizar la medición de la variable
  // Si retorna 0 significa que hubo un error
  status = presion.startTemperature();
  if (status != 0)
  {

    // Espera el tiempo necesario para capturar el valor de temperatura
    delay(status);
    status = presion.getTemperature(temperatura_BMP180);
    if (status != 0)
    {
      // Imprime el valor de temperatura
      Serial.print("Temperatura BMP180: ");
      Serial.print(temperatura_BMP180, 2);
      Serial.println(" °C");

      // Inicia la medición de presión
      // El parámetro que se coloca es el ajuste de sobremuestreo
      // De 0 a 3, (Una mayor resolución implica una espera más larga)

      status = presion.startPressure(3);
      if (status != 0)
      {
        // Espera para completar la medición
        delay(status);

        // Para obtener la presión, anteriormente se debió capturar el valor de temperautra
        // Si la temperatura es estable, se puede realizar una medición de temperatura por varias de presión

        status = presion.getPressure(presion_BMP180, temperatura_BMP180);
        if (status != 0)
        {
          // Imprime el valor de presión
          Serial.print("Presión absoluta: ");
          Serial.print(presion_BMP180, 2);
          Serial.println(" mb");

          // El sensor de presión devuelve una presión absoluta, que varía con la altitud.
          // Para eliminar los efectos de la altitud, se utiliza la función del nivel del mar y la altitud actual.
          // Este número se usa comúnmente en los informes meteorológicos.
          // Parámetros: presion_BMP180 = presión absoluta en mb, ALTURA_MAR = Altura actual en m.
          // Resultado: p0 = presión compensada al nivel del mar en mb

          p0 = presion.sealevel(presion_BMP180, ALTURA_MAR); // 20 metros sobre el nivel del mar en invemar
          Serial.print("Presión relativa al nivel del mar: ");
          Serial.print(p0, 2);
          Serial.println(" mb");

          // Por otro lado, si se desea determinar la altura a partir de la lectura de presión,
          // se usa la función de altitud junto con una presión de referencia (al nivel del mar u otro).
          // Parámetros: presion_BMP180 = Presión absoluta en mb, p0 = Presión compensada en mb.
          // Resultado: altura = altitud en m.

          altura = presion.altitude(presion_BMP180, p0);
          Serial.print("Altura computada: ");
          Serial.print(altura, 0);
          Serial.println(" metros");
        }
        else
          Serial.println("Error al realizar al medición de presión\n");
      }
      else
        Serial.println("Error al iniciar la medición de presión\n");
    }
    else
      Serial.println("Error al realizar al medición de temperatura\n");
  }
  else
    Serial.println("Error al iniciar la medición de temperatura\n");

  /* ************** SENSOR DS18B20 ************** */
  // Variables que controlan la lectura de temperatura
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];
  float temperatura_DS18B20;

  if (!ds.search(addr))
  {
    ds.reset_search();
    delay(250);
    return;
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1); // start conversion, with parasite power on at the end

  delay(1000); // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.

  present = ds.reset();
  ds.select(addr);
  ds.write(0xBE); // Read Scratchpad

  for (i = 0; i < 9; i++)
  { // we need 9 bytes
    data[i] = ds.read();
  }

  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s)
  {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10)
    {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  }
  else
  {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00)
      raw = raw & ~7; // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20)
      raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40)
      raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  temperatura_DS18B20 = (float)raw / 16.0;
  Serial.println();
  Serial.print("Temperatura DS18B20 = ");
  Serial.print(temperatura_DS18B20);
  Serial.println(" °C");

  /* ************** RELOJ ************** */

  DateTime now = rtc.now();
  Serial.print(now.year(), DEC);
  Serial.print('/');
  Serial.print(now.month(), DEC);
  Serial.print('/');
  Serial.print(now.day(), DEC);
  Serial.print(" ");
  Serial.print(now.hour(), DEC);
  Serial.print(':');
  Serial.print(now.minute(), DEC);
  Serial.print(':');
  Serial.print(now.second(), DEC);
  Serial.println();

  /* ************** Memoria SD ************** */
  // Cadena de datos donde se ensamblarán los datos
  String dataString = "";
  String cadena = "Fecha,Temperatura DHT,Humedad DHT,Altura posición,Temperatura BMP180,Presión BMP180,Presión relativa,Altura,Temperatura DS18B20";

  dataString += String(now.year());
  dataString += "/";
  dataString += String(now.month());
  dataString += "/";
  dataString += String(now.day());
  dataString += " ";
  dataString += String(now.hour());
  dataString += ":";
  dataString += String(now.minute());
  dataString += ":";
  dataString += String(now.second());
  dataString += ",";
  dataString += String(temperatura_DHT);
  dataString += ",";
  dataString += String(humedad_DHT);
  dataString += ",";
  dataString += String(ALTURA_MAR);
  dataString += ",";
  dataString += String(temperatura_BMP180);
  dataString += ",";
  dataString += String(presion_BMP180);
  dataString += ",";
  dataString += String(p0);
  dataString += ",";
  dataString += String(altura);
  dataString += ",";
  dataString += String(temperatura_DS18B20);

  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  archivo = SD.open("tesoro.txt", FILE_WRITE);

  // if the file is available, write to it:
  if (archivo)
  {
    archivo.println(dataString);
    archivo.close();
    // print to the serial port too:
    Serial.println(cadena);
    Serial.println(dataString);
    digitalWrite(led, LOW);
  }
  // if the file isn't open, pop up an error:
  else
  {
    Serial.println("Error al abrir el archivo");
    digitalWrite(led, HIGH);
  }
  delay(600000); // Pausa por 10 segundos.
}
