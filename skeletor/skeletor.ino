#include <Servo.h> 

Servo servo;  // create servo object to control a servo 

const int pirPin = A0;
const int eyesPin = 5;
const int motorPin = 6;
const int servoPin = 9;

void setup() {
  Serial.begin(9600);
  
  pinMode(pirPin, INPUT);
  digitalWrite(pirPin, LOW);
  
  pinMode(eyesPin, OUTPUT);
  digitalWrite(eyesPin, LOW);
  pinMode(motorPin, OUTPUT);
  digitalWrite(motorPin, LOW);

  servo.attach(servoPin);
}

void loop() {
  digitalWrite(motorPin, HIGH);
  analogWrite(eyesPin, 30);
  servo.write(170);
  while (digitalRead(pirPin) == LOW) {
    delay(100);
  }
  servo.write(100);
  digitalWrite(motorPin, LOW);
  analogWrite(eyesPin, 255);
  do {
    delay(4000);
  } while (digitalRead(pirPin) == HIGH);
}

