#include <SoftwareSerial.h>

// These constants won't change.  They're used to give names
// to the pins used:
const int rangePowerPin = A0;
const int rangeFrequencyInPin = A1;
const int rangeAnalogInPin = A2;
const int pirSensorPin = A3;
const int ledGreenPin = 9;
const int ledBluePin = 10;
const int ledRedPin = 11;
const int ledGroundPin = 12;

const int bluetoothPowerPin = 3;
const int bluetoothTxPin = 5;
const int bluetoothRxPin = 6;

int sensorValue = 0;        // value read from the pot

int calibrationData[100];

SoftwareSerial bluetooth(bluetoothRxPin, bluetoothTxPin);

void setup() {
  // initialize serial communications at 9600 bps:
  Serial.begin(9600);

  bluetooth.begin(4800);

  pinMode(rangePowerPin, OUTPUT);
  digitalWrite(rangePowerPin, HIGH);
  pinMode(rangeAnalogInPin, INPUT);
  pinMode(rangeFrequencyInPin, INPUT);
  
  pinMode(ledGreenPin, OUTPUT);
  pinMode(ledBluePin, OUTPUT);
  pinMode(ledGreenPin, OUTPUT);
  pinMode(ledGroundPin, OUTPUT);
  digitalWrite(ledGroundPin, LOW);

  pinMode(bluetoothPowerPin, OUTPUT);
  digitalWrite(bluetoothPowerPin, HIGH);

  pinMode(pirSensorPin, INPUT);
  // Wait 15 seconds for PIR sensor to calibrate.
  for (int i = 0; i < 300; i++) {
    setRgb(mymap(i, 80, 160, 0, 255), mymap(i, 160, 200, 0, 255), mymap(i, 0, 80, 0, 255));
    delay(50);
    bluetooth.println("Setup");
  }

/*
  for (int i = 0; i < 200; i++) {
    sensorValue = analogRead(rangeAnalogInPin);
    if (i > 100) {
      calibrationData[i - 100] = sensorValue;
    }
    setRgb(mymap(i, 80, 160, 0, 255), mymap(i, 160, 200, 0, 255), mymap(i, 0, 80, 0, 255));

    if (i % 10 == 0) {
     Serial.print("calibration ");
     Serial.println(i);
    }
    delay(100);
  }
  quickSort(calibrationData, 0, 99);
  Serial.print("min = ");
  Serial.print(calibrationData[0]);
  Serial.print(", 25th = ");
  Serial.print(calibrationData[25]);
  Serial.print(", median = ");
  Serial.print(calibrationData[49]);
  Serial.print(", 75th = ");
  Serial.print(calibrationData[75]);
  Serial.print(", 90th = ");
  Serial.print(calibrationData[90]);
  Serial.print(", 95th = ");
  Serial.print(calibrationData[95]);
  Serial.print(", max = ");
  Serial.println(calibrationData[99]);
  */
}

const int movementSize = 60;
int movements[movementSize];
int movementIndex = 0;
unsigned long lastFrontMovement = 0;
unsigned long lastLoopTime = 0;

int testingMode = 1;

int countMovements(int first, int last) {
  int total = 0;
  for (int i = first; i < last; i++) {
    total += movements[(movementIndex + movementSize + i) % movementSize];
  }
  return total;
}

void loop() {
  long combinedFreq = getFrequency(rangeFrequencyInPin);
  int sensorValue = analogRead(rangeAnalogInPin);            
  int frontMovement = analogRead(pirSensorPin);

  bluetooth.println("Loop");

  Serial.print("front (PIR) = ");
  Serial.print(frontMovement);
  Serial.print(", combined (doplar) = ");
  Serial.print(sensorValue);
  Serial.print(", freq = ");
  Serial.print(combinedFreq);
  Serial.print(", 0-5 = ");
  Serial.print(countMovements(-5, 0));
  Serial.print(", 5-20 = ");
  Serial.print(countMovements(-20, -5));
  Serial.print(", 5-30 = ");
  Serial.println(countMovements(-30, -5));

  movementIndex = (movementIndex + 1) % movementSize;
  if (frontMovement) {
    lastFrontMovement = millis();
    movements[movementIndex] = 0;
  } else {
    movements[movementIndex] = (combinedFreq > 100) ? 1 : 0;
  }

  if (testingMode) {
    if (frontMovement) {
        setRgb(0, 0, 255);  // blue
    } else {
      if (countMovements(-20, -5) > 0) {
        setRgb(255, 0, 0);  // red
      } else if (countMovements(-30, -5) > 1) {
        setRgb(80, 10, 0);  // yellow
      } else {
        setRgb(0, 255, 0);  // green
      }
    }
  } else {  
    if ((millis() - lastFrontMovement) < 15000) {
      if (countMovements(-20, -5) > 1) {
        setRgb(255, 0, 0);  // red
      } else if (countMovements(-30, -5) > 1) {
        setRgb(80, 10, 0);  // yellow
      } else {
        setRgb(0, 255, 0);  // green
      }                    
    } else {
      setRgb(0, 0, 0);  // gray
    }
  }

  Serial.println(millis());
  while (millis() < lastLoopTime + 100) {
    delay(10);
  }
  lastLoopTime = millis();
}

int lastRed = 0;

/*
//  int normalized = mymap(sensorValue, calibrationData[60], calibrationData[90], 0, 255);  
  if (sensorValue > calibrationData[90]) {
    setRgb(255, 0, 0);  // red
  } else if (sensorValue > calibrationData[60]) {
    setRgb(80, 10, 0);  // yellow
  } else if (sensorValue > 0) {
    setRgb(0, 255, 0);  // green
  } else {
    setRgb(5, 5, 5);  // gray
  }                    
*/

void loop_old() {
  setRgb(0, 255, 0);
  long pulse = pulseIn(rangeFrequencyInPin, HIGH, 100000);
  Serial.print("lastRed = " );                       
  Serial.print(lastRed);      
  Serial.print(", pulse = " );                       
  Serial.println(pulse);      
  
  if (pulse > 0) {
    setRgb(255, 0, 0);  // red
    lastRed = 100;
  } else {
    if (lastRed > 0) {
      lastRed--;
      if (lastRed == 0) {
        setRgb(0, 255, 0);
      } else if (lastRed < 5) {
        setRgb(80, 10, 0);  // yellow
      }
    }
  }
  delay(50);
}

long getFrequency(int pin) {
  #define SAMPLES 1
  long freq = 0;
  for(unsigned int j=0; j<SAMPLES; j++) freq+= 500000/pulseIn(pin, HIGH, 100000);
  return freq / SAMPLES;
}

void setRgb(int r, int g, int b) {
  analogWrite(ledGreenPin, g);
  analogWrite(ledBluePin, b);
  analogWrite(ledRedPin, r);
}

void quickSort(int arr[], int left, int right) {
      int i = left, j = right;
      int tmp;
      int pivot = arr[(left + right) / 2];
 
      /* partition */
      while (i <= j) {
            while (arr[i] < pivot)
                  i++;
            while (arr[j] > pivot)
                  j--;
            if (i <= j) {
                  tmp = arr[i];
                  arr[i] = arr[j];
                  arr[j] = tmp;
                  i++;
                  j--;
            }
      };
 
      /* recursion */
      if (left < j)
            quickSort(arr, left, j);
      if (i < right)
            quickSort(arr, i, right);
}

int mymap(int x, int a, int b, int c, int d) {
  if (x < a) {
    return c;
  }
  if (x > b) {
    return d;
  }
  return map(x, a, b, c, d);
}
