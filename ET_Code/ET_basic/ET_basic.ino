void setup() {
  // put your setup code here, to run once:
  pinMode(25, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(25, HIGH);
  delay(1000);
  digitalWrite(25, LOW);
  delay(1000);
}
