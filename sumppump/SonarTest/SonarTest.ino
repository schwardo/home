#include <SoftwareSerial.h>

#include <NewPing.h>

#define TRIGGER_PIN  12  // Arduino pin tied to trigger pin on the ultrasonic sensor.
#define ECHO_PIN     11  // Arduino pin tied to echo pin on the ultrasonic sensor.
#define MAX_DISTANCE 200 // Maximum distance we want to ping for (in centimeters). Maximum sensor distance is rated at 400-500cm.

NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE); // NewPing setup of pins and maximum distance.

SoftwareSerial mySerial(2, 3);

void setup()  
{
  // set the data rate for the SoftwareSerial port
  mySerial.begin(9600);
}

void loop() // run over and over
{
  float inches = sonar.ping_in();

  mySerial.write(254); // move cursor to beginning of first line
  mySerial.write(128);

  mySerial.write("                    "); // clear display
  mySerial.write("                    ");
  mySerial.write("                    ");
  mySerial.write("                    ");

  mySerial.write(254); // move cursor to beginning of first line
  mySerial.write(128);
  mySerial.print("Dist: "); // Convert ping time to distance in cm and print result (0 = outside set distance range)
  mySerial.print(inches / 12.0); // Convert ping time to distance in cm and print result (0 = outside set distance range)
  mySerial.print(" ft");
  delay(1000);
}
