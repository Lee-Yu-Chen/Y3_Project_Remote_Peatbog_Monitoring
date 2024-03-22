void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(37,OUTPUT);

}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(37,HIGH);
  delay(1000);
  /*
  digitalWrite(37,LOW);
  delay(1000);

  
  analogWrite(37,60);
  delay(1000);
  
  analogWrite(37,0);
  delay(1000);*/

}
