// Adafruit Motor shield library
// copyright Adafruit Industries LLC, 2009
// this code is public domain, enjoy!

#include <AFMotor.h>
#include <Servo.h> 

AF_DCMotor motor1(3);
AF_DCMotor motor2(4);
// DC hobby servo
Servo servo1;
Servo servo2;

void setup() {
  Serial.begin(9600);           // set up Serial library at 9600 bps
  Serial.println("Motor party!");
  
  // turn on motor #2
  motor1.setSpeed(200);
  motor1.run(RELEASE);
  motor2.setSpeed(200);
  motor2.run(RELEASE);
}

int i;

// Test the DC motor, stepper and servo ALL AT ONCE!
void loop() {
  servo2.attach(9);
  servo2.attach(10);

  servo1.write(128);
  servo2.write(128);
  motor1.run(RELEASE);
  motor2.run(RELEASE);

  delay(1000);

  servo1.detach();
  servo2.detach();

  delay(4000);

  servo2.attach(10);

  for (i=0; i<255; i += 25) {
    servo2.write(i);
    delay(500);
  }
  servo2.write(128);
  
  motor1.run(BACKWARD);
  motor2.run(BACKWARD);
  delay(1000);
 
  motor1.run(FORWARD);
  motor2.run(FORWARD);
  delay(1000);
}
