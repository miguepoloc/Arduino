// Código para el nodo de Sistemas Inteligentes

// Librería para el sensor de temperatura del suelo
#include <Adafruit_MAX31865.h>

// Se incluyen las librerías de la SD
#include <SPI.h>
#include <SD.h>
#include "FS.h"


// Pin de conexión SPI para la PT100
#define CS_PT100 2

Adafruit_MAX31865 thermo = Adafruit_MAX31865(CS_PT100);

// Valor de referencia de la resistencia. Se usa 430.0 para PT100 y 4300.0 para PT1000
#define RREF 430.0
// El valor nominal a 0°C de la resistencia es de 100 para PT100 y 1.000 para PT1000
#define RNOMINAL  100.0

float valor_pt100;

void pt100() {
  uint16_t rtd = thermo.readRTD();

  Serial.print("RTD valor: "); Serial.println(rtd);
  float ratio = rtd;
  ratio /= 32768;
  Serial.print("Ratio = "); Serial.println(ratio, 8);
  Serial.print("Resistencia = "); Serial.println(RREF * ratio, 8);
  valor_pt100 = thermo.temperature(RNOMINAL, RREF);
  Serial.print("Temperatura = "); Serial.println(valor_pt100);

  // Check and print any faults
  uint8_t fault = thermo.readFault();
  if (fault) {
    Serial.print("Fault 0x"); Serial.println(fault, HEX);
    if (fault & MAX31865_FAULT_HIGHTHRESH) {
      Serial.println("RTD High Threshold");
    }
    if (fault & MAX31865_FAULT_LOWTHRESH) {
      Serial.println("RTD Low Threshold");
    }
    if (fault & MAX31865_FAULT_REFINLOW) {
      Serial.println("REFIN- > 0.85 x Bias");
    }
    if (fault & MAX31865_FAULT_REFINHIGH) {
      Serial.println("REFIN- < 0.85 x Bias - FORCE- open");
    }
    if (fault & MAX31865_FAULT_RTDINLOW) {
      Serial.println("RTDIN- < 0.85 x Bias - FORCE- open");
    }
    if (fault & MAX31865_FAULT_OVUV) {
      Serial.println("Under/Over voltage");
    }
    thermo.clearFault();
  }
  Serial.println();
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
void setup() {
  Serial.begin(9600);
  Serial.println("Adafruit MAX31865 PT100 con almacenamiento en SD");

  thermo.begin(MAX31865_3WIRE);  // set to 2WIRE or 4WIRE as necessary
  Serial.print("Inicializando SD card...");

  if (!SD.begin(5)) {
    Serial.println("Inicialización fallida!");
    while (1);
  }
  Serial.println("Inicialización lista.");


}


char Buf[50];
void loop() {
  String msg = "{\"temperatura\": ";
  pt100();
  Serial.print("Prueba = "); Serial.println(valor_pt100);
  // Serial.print("Función Temperatura = "); Serial.println(valor_pt100);
  msg.concat(String(valor_pt100));
  msg.concat(", \"humedad\":14}");
  
  msg.toCharArray(Buf, 50);
  writeFile(SD, "/prueba2.json", msg);
  delay(10000);
}
