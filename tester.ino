char latch_pin =3;
char shift_pin = 2;
char serial_pin = 4;
int cont=0;
void setup(){
  pinMode(latch_pin,OUTPUT);
  pinMode(shift_pin,OUTPUT);
  pinMode(serial_pin,OUTPUT);
  PORTD =0;
  Serial.begin(9600);
}
int dilei=100;
void loop(){
  if (Serial.available()){
    digitalWrite(serial_pin,((Serial.read() =='0') ? 0 : 1));
  }
  cont++;
  digitalWrite(latch_pin,HIGH);
  digitalWrite(shift_pin,HIGH);
  delay(dilei);
  digitalWrite(latch_pin,LOW);
  digitalWrite(shift_pin,LOW);
  delay(dilei);
}
