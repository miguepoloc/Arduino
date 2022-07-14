// Se incluyen las librerías
#include <SPI.h>
#include <SD.h>
#include "FS.h"

void writeFile(fs::FS &fs, const char * path, const char * mensaje){
  Serial.printf("Escribiendo el archivo: %s\n", path);

  File file = fs.open(path, FILE_WRITE);
  if(!file){
    Serial.println("Error al abrir el archivo para escribirlo");
    return;
  }
  if(file.print(mensaje)){
    Serial.println("Archivo escrito");
  } else {
    Serial.println("Error al escribir");
  }
  file.close();
}

void deleteFile(fs::FS &fs, const char * path){
  Serial.printf("Borrando el archivo: %s\n", path);
  if(fs.remove(path)){
    Serial.println("Archivo borrado");
  } else {
    Serial.println("Error al borrar el archivo");
  }
}

void setup() {
  // Se abren las comunicaciones por el puerto serial:
  Serial.begin(9600);
  while (!Serial) {
    ; // Espera hasta que el puerto serial se conecte
  }


  Serial.print("Inicializando SD card...");

  if (!SD.begin(5)) {
    Serial.println("Inicialización fallida!");
    while (1);
  }
  Serial.println("Inicialización lista.");
  
  writeFile(SD, "/hello2.txt", "{\"temperatura\": 12, \"humedad\":14}");

}

void loop() {
  // No pasa nada.
}
