/*------------------------------------------------------
  Programa para buscar dispositivos I2C conectados.
  Manda las direcciones que encuentra a la puerta serie
---------------------------------------------------------
*/

#include <Wire.h>
 
void setup()
{
  Wire.begin();
 
  Serial.begin(9600);
  Serial.println("\nI2C Escaner");
}
 
void loop()
{
  byte error, address;
  int nDevices;
 
  Serial.println("Escaneando...");
 
  nDevices = 0;
  for(address = 1; address < 127; address++ )
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    Wire.beginTransmission(address);
    error = Wire.endTransmission();
 
    if (error == 0)
    {
      Serial.print("Dispositivo I2C encontrado en la dirección 0x");
      if (address<16)
        Serial.print("0");
      Serial.print(address,HEX);
      Serial.println("  !");
 
      nDevices++;
    }
    else if (error==4)
    {
      Serial.print("Error desconocido en la dirección 0x");
      if (address<16)
        Serial.print("0");
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0)
    Serial.println("No se encontraron dispositivos I2C\n");
  else
    Serial.println("Listo\n");
 
  delay(5000);           // wait 5 seconds for next scan
}
