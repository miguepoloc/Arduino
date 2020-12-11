/*
***************************************************
  Programmed by iakbasnayaka for www.setnfix.com
  This is for checking of AT commands for SIM800L
  https://www.youtube.com/channel/UCM9NQ4WcSbzTiCdoyOvzA1g
***************************************************
*/

#include <SoftwareSerial.h>
SoftwareSerial Sim800l(10, 11);
void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Sim800l.begin(9600);
  Sim800l.println("AT&F\r\n");
  delay(1000);
  Sim800l.println("AT+CLIP=1\r\n");
  delay(1000);
  Sim800l.println("AT+CNMI=1,0,0,0,0\r\n");
  delay(1000);
  //Se establece el formato de SMS en ASCII
  Sim800l.println("AT+CMGF=1\r\n");
  delay(1000);
}

void loop() {

  while (Sim800l.available()) {
    Serial.write(Sim800l.read());
  }


  while (Serial.available()) {
    Sim800l.write(Serial.read());
  }

  //Sim800l.print("AT+CMGL=\"ALL\"\r\n");
 // delay(5000);

}
