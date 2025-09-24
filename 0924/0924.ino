void setup() {
  // put your setup code here, to run once:
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(3, LOW);// turn the B LED on
  digitalWrite(4, LOW);// turn the R LED on
  delay(500);// wait for a second
  digitalWrite(3, HIGH);// turn the B LED off
  digitalWrite(4, HIGH); //r
  delay(500);// wait for a second
  digitalWrite(3, LOW);// turn the b LED on
  delay(500);// wait for a second
  digitalWrite(3, HIGH);// turn the b LED off
  delay(500);// wait for a second
  digitalWrite(5, LOW);// turn the g LED on
  delay(500);// wait for a second
  digitalWrite(5, HIGH);// turn the g LED off
  delay(500);// wait for a second
  digitalWrite(5, LOW);// turn the g LED on
  digitalWrite(4,LOW);
  delay(500);// wait for a second
  digitalWrite(5, HIGH);// turn the g LED off
  digitalWrite(4, HIGH);
  delay(500);// wait for a second
}
