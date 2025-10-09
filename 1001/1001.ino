const int ButtonPin = 2;
const int RledPin = 3;
const int GledPin = 4;
const int BledPin = 5;

int buttonState = 0;
int ledcolor = 0;

bool ButtonPressed = false;

void setup() {
  // put your setup code here, to run once:
  pinMode(RledPin, OUTPUT);
  pinMode(GledPin, OUTPUT);
  pinMode(BledPin, OUTPUT);
  pinMode(ButtonPin, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  buttonState = digitalRead(ButtonPin);

  if(buttonState == HIGH && ! ButtonPressed){
    ledcolor = ledcolor+1;
    ButtonPressed = true;
    //delay(100);
  }
  if(buttonState == LOW){
    ButtonPressed = false;
    //delay(100);
  }
  if(ledcolor == 0){
    digitalWrite(RledPin, HIGH);
    digitalWrite(BledPin, HIGH);
    digitalWrite(GledPin, HIGH);
  }
  else if(ledcolor == 1){
    digitalWrite(RledPin, LOW);
    digitalWrite(BledPin, HIGH);
    digitalWrite(GledPin, HIGH);
  }
  else if(ledcolor == 2){
    digitalWrite(RledPin, HIGH);
    digitalWrite(BledPin, LOW);
    digitalWrite(GledPin, HIGH);
  }
  else if(ledcolor == 3){
    digitalWrite(RledPin, HIGH);
    digitalWrite(BledPin, HIGH);
    digitalWrite(GledPin, LOW);
  }
  else if(ledcolor == 4){
    digitalWrite(RledPin, LOW);
    digitalWrite(BledPin, LOW);
    digitalWrite(GledPin, HIGH);
  }
  else if(ledcolor == 5){
    digitalWrite(RledPin, LOW);
    digitalWrite(BledPin, HIGH);
    digitalWrite(GledPin, LOW);
  }
  else if(ledcolor == 6){
    digitalWrite(RledPin, HIGH);
    digitalWrite(BledPin, LOW);
    digitalWrite(GledPin, LOW);
  }
  else if(ledcolor == 7){
    digitalWrite(RledPin, LOW);
    digitalWrite(BledPin, LOW);
    digitalWrite(GledPin, LOW);
  }
  else if(ledcolor == 8){
    ledcolor = 1;
  }

}
