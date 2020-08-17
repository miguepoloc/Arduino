/*
  SD card

  Este ejemplo crea y borra un archivo en la SD
  Circuito:
   SD card usa el protocolo SPI:
    SPI   - UNO     - SPI - Mega
 ** MOSI  - pin 11  - DI  - pin 51
 ** MISO  - pin 12  - DO  - pin 50
 ** CLK   - pin 13  - SCK - pin 52
 ** CS    - pin 4   - CS  - pin 53

*/
// Se incluyen las librerías
#include <SPI.h>
#include <SD.h>

// Se declara la variable archivo
File archivo;

void setup() {
  // Se abren las comunicaciones por el puerto serial:
  Serial.begin(9600);
  while (!Serial) {
    ; // Espera hasta que el puerto serial se conecte
  }


  Serial.print("Inicializando SD card...");

  if (!SD.begin(53)) {
    Serial.println("Inicialización fallida!");
    while (1);
  }
  Serial.println("Inicialización lista.");

  //Si existe el archivo
  if (SD.exists("example.txt")) {
    Serial.println("El archivo example.txt existe.");
  } else {
    Serial.println("El archivo example.txt no existe.");
  }

  // Abre un nuevo archivo e inmediatamente lo cierra:
  Serial.println("Creando example.txt...");
  archivo = SD.open("example.txt", FILE_WRITE);
  archivo.close();

  // Revisa si el archivo existe:
  if (SD.exists("example.txt")) {
    Serial.println("El archivo example.txt existe.");
  } else {
    Serial.println("El archivo example.txt no existe.");
  }

  // Elimina el archivo:
  Serial.println("Eliminando el archivo example.txt...");
  SD.remove("example.txt");

  if (SD.exists("example.txt")) {
    Serial.println("El archivo example.txt existe.");
  } else {
    Serial.println("El archivo example.txt no existe.");
  }
}

void loop() {
  // No pasa nada.
}
