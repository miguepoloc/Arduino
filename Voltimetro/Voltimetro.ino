//const float R1 = 21570; //Poner valores reales (medidos) de R1, R2 y shunt para hacer más precisa a la medición
//const float R2 = 12020;
const float R1 = 98040; //Poner valores reales (medidos) de R1, R2 y shunt para hacer más precisa a la medición
const float R2 = 9837;
const float Rf = R1 + R2;
float x;
const float shunt = 1.5;
float vin, vout, input, vin2 = 0, iin, iin2, potencia, ampe;
float entradaV = A0, entradaI = A1;

void setup() {
  Serial.begin(115200);
  analogReference(INTERNAL1V1);
  pinMode (entradaV, INPUT);
  pinMode (entradaI, INPUT);
}

void loop() {
  volt();
  //  //amp();
  //  //power(); simplemente agregué la medición de potencia, si no la desean dejen esto como está, si la desean simplemente borren las "//" y este comentario.
  delay(1000);
}

void volt() {
  for (int i = 1; i <= 150; i++)
  {
    input = analogRead (entradaV);
    vout = (input * 1.1) / 1024;
    vin = vout / (R2 / (R1 + R2));
    vin2 = vin2 + vin;
  }
  vin = vin2 / 150;
  ampe = vin / Rf * 1000;
  vin2 = 0;
  Serial.print (vin);
  Serial.print ("V");
  Serial.print ("\n");
  Serial.print (ampe);
  Serial.print ("mA");
  Serial.print ("\n");
}

void amp() {
  for (int i = 1; i <= 150; i  )
  {
    input = analogRead (entradaI);
    vout = (input * 1.1) / 1024;
    iin = vout / shunt;
    iin2 = iin2 + iin;
  }
  iin = iin2 / 150;
  iin2 = 0;
  if (iin <= 0.005) {
    iin = 0;
  }
  Serial.print (iin);
  Serial.print ("A");
  Serial.print ("\n");
}

void power() {
  potencia = vin * iin;
  Serial.print (potencia);
  Serial.print ("W");
  Serial.print ("\n");
}
