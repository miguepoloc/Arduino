/*
  Voltimetro

  Este código lee el voltaje de entrada de un pin analógico el cual ha pasado por un divisor de tensión
  que permite saber cuál es el voltaje de entrada.
  Adicionalmente se conecta el sensor ACS712ELCTR-30A-T para monitorear la corriente

  Circuito:
  Vin-------R1-----|Vout
     i--->         |
                   |
                   R2
                   |
                   o----
                        Sensor de corriente
                   o----
                   |
                 Tierra
*/

//Poner valores reales (medidos) de R1, R2
const float R1 = 98040;
const float R2 = 9837;
const float Rf = R1 + R2;
float x;
float vin, vout, input, vin2 = 0, iin, iin2, potencia, i;
float entradaV = A0;

void setup() {
  //Se inicia la comunicación serial
  Serial.begin(9600);
  //Se cambia el voltaje de referencia a 1.1V para mayor resolución
  analogReference(INTERNAL1V1);
  //Se coloca el pun A0 como entrada
  pinMode (entradaV, INPUT);
}

void loop() {
  volt();
  delay(1000);
}

void volt() {
  // Se realiza la medición 150 veces
  for (int j = 1; j <= 150; j++)
  {
    // Lee el voltaje de A0
    input = analogRead (entradaV);
    // Se convierte el voltaje de entrada a bits
    vout = (input * 1.1) / 1024;
    // Se realiza la conversión del voltaje de salida para saber cuánto entra
    vin = vout / (R2 / (R1 + R2));
    // Se suma el voltaje 150 veces para luego realizar el promedio
    vin2 = vin2 + vin;
  }
  // Se promedia el voltaje de entrada
  vin = vin2 / 150;
  // Se calcula la corriente
  i = vin / Rf * 1000;
  vin2 = 0;
  Serial.print (vin);
  Serial.print ("V");
  Serial.print ("\n");
  Serial.print (i);
  Serial.print ("mA");
  Serial.print ("\n");
  // Se calcula la potencia
  potencia = vin * i;
  Serial.print (potencia);
  Serial.print ("W");
  Serial.print ("\n");
}
