/*
  SD card

  Este ejemplo escribe sobre un archivo en la SD
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

  // Abre el archivo. Cabe aclarar que sólo se puede abrir un archivo al mismo tiempo,
  // entonces debes cerrar algún otro antes de abrir uno.
  archivo = SD.open("test.txt", FILE_WRITE);

  // Si el archivo abrió bien:
  if (archivo) {
    Serial.print("Escribiento en test.txt...");
    archivo.println("Probando 1, 2, 3");
    // Cierra el archivo:
    archivo.close();
    Serial.println("Listo.");
  } else {
    // Si el archivo no abre, imprime un error:
    Serial.println("Error abriendo test.txt");
  }

  // Vuelve a abrir el archivo para su lectura:
  archivo = SD.open("test.txt", FILE_READ);
  if (archivo) {
    Serial.println("test.txt:");

    // Leer del archivo hasta que no haya más nada en él:
    char c = archivo.read();
    while (c != -1) //Leemos hasta el final
    {
      Serial.print(c);
      c = archivo.read();
    }
    // Cierra el archivo:
    archivo.close();
  } else {
    // Si el archivo no abre, imprime un error:
    Serial.println("Error abriendo test.txt");
  }
}

void loop() {
  // No se hace más nada
}
