void setup() {
  
  Serial.begin(9600);
}

void loop() {
  
  float voltajeSensor =get_voltage(1000000);//obtenemos voltaje del sensor(10000 muestras) 
  Serial.print("Voltaje del sensor: ");
  Serial.println(voltajeSensor ,3);     
}


float get_voltage(int n_muestras)
{
  float voltage=0;
  
  for(int i=0;i<n_muestras;i++)
  {
    voltage =voltage+analogRead(A1) * (5.0 / 1023.0);    
  }
  voltage=voltage/n_muestras;
  return(voltage);
}
