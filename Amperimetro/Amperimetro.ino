//float Sensibilidad = 0.066; //sensibilidad en Voltios/Amperio para sensor de 5A
float Sensibilidad = 0.26; //sensibilidad en Voltios/Amperio para sensor de 5A

void setup() {

  Serial.begin(9600);
}

void loop() {

  float I = get_corriente(200); //obtenemos la corriente promedio de 500 muestras
  Serial.print("Corriente: ");
  Serial.print(I * 1000, 3);
  Serial.println(" mA");
  delay(1000);
}

float get_corriente(int n_muestras)
{
  float voltajeSensor;
  float corriente = 0;
  for (int i = 0; i < n_muestras; i++)
  {
    voltajeSensor = analogRead(A1) * (5.0 / 1023.0);////lectura del sensor
    corriente = corriente + (voltajeSensor - 2.492) / (Sensibilidad * -1); //Ecuación  para obtener la corriente
  }
  Serial.print("Voltaje sensor: ");
  Serial.print(voltajeSensor, 5);
  Serial.println(" V");
  corriente = corriente / n_muestras;
  return (corriente);
}
