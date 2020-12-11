/*
  ARDUINO UNO SIM800L
  D7 TX
  D8 RX
  GND GND
*/
#include <SoftwareSerial.h>
//Se declara los pines en el arduino Uno
SoftwareSerial MOD_SIM800L(10, 11);
void setup() {
  //Se establece la velocidad para el puerto serie
  Serial.begin(9600);
  while (!Serial);

  //Velocidad de trabajo entre el Arduino Uno y el Modulo SIM800L
  MOD_SIM800L.begin(9600);
  delay(1000);
  Serial.println("Configuracion Completa!");
  Serial.println("Enviando SMS...");

  MOD_SIM800L.println("AT&F\r\n");
  delay(1000);
  MOD_SIM800L.println("AT+CLIP=1\r\n");
  delay(1000);
  MOD_SIM800L.println("AT+CNMI=1,0,0,0,0\r\n");
  delay(1000);
  //Se establece el formato de SMS en ASCII
  MOD_SIM800L.println("AT+CMGF=1\r\n");
  delay(1000);

  //Enviar comando para un nuevos SMS al numero establecido
  //    MOD_SIM800L.println("AT+CMGS=\"+573003859853\"\r\n");
  MOD_SIM800L.println("AT+CMGS=\"+573003859853\"");

  delay(100);

  //Enviar contenido del SMS
  MOD_SIM800L.println("Hola prueba desde el arduino.");
  delay(1000);

  //Enviar Ctrl+Z
  MOD_SIM800L.println((char)26);
  delay(100);

  MOD_SIM800L.println();
  Serial.println("Mensaje enviado!");

}
void loop() {
  while (MOD_SIM800L.available()) {
    Serial.write(MOD_SIM800L.read());
  }


  while (Serial.available()) {
    MOD_SIM800L.write(Serial.read());
  }
}
