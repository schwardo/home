#include <Adafruit_NeoPixel.h>

const int stripPin = 53;
const int redPins[] = { 11, 8, 5, 2 };
const int greenPins[] = { 12, 9, 6, 3 };
const int bluePins[] = { 13, 10, 7, 4 };
const int ledGndPins[] = { 47, 45, 43, 41 };
const int switchPins[] = { 52, 44, 46, 50 };
const int switchGndPins[] = {29, 27, 25, 23};

// Parameter 1 = number of pixels in strip
// Parameter 2 = pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
Adafruit_NeoPixel strip = Adafruit_NeoPixel(80, stripPin, NEO_GRB + NEO_KHZ800);

const int stripLength = 5;
//const int stripPixels[] = { 0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60, 65, 70, 75 };
const int stripPixels[] = { -1, -1, 12, -1,
                            -1,  9, 10, 11,
                             8,  7,  6,  5,
                             0,  1,  2,  3};

const int numColors = 8;
const int redPalette[] = { 0, 255, 255, 0, 0, 255, 0, 255 };
const int greenPalette[] = { 0, 255, 0, 255, 0, 255, 255, 0 };
const int bluePalette[] = { 0, 255, 0, 0, 255, 0, 255, 255 };
uint32_t stripPalette[numColors];
uint32_t stripBlack = strip.Color(0, 0, 0);

int buttonColors[16];
int buttonTimes[16];

void setup() {
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

   for (int i = 0; i < 4; i++) {
     pinMode(redPins[i], OUTPUT);
     pinMode(greenPins[i], OUTPUT);
     pinMode(bluePins[i], OUTPUT);

     pinMode(ledGndPins[i], OUTPUT);
     digitalWrite(ledGndPins[i], HIGH);

     pinMode(switchPins[i], INPUT);     
     digitalWrite(switchPins[i], HIGH);

     pinMode(switchGndPins[i], OUTPUT);     
     digitalWrite(switchGndPins[i], HIGH);
   }
   
   for (int i = 0; i < numColors; i++) {
     stripPalette[i] = strip.Color(redPalette[i], bluePalette[i], greenPalette[i]);
   }
}

void loop() {
  int now = millis();

  for (int row = 0; row < 4; row++) {
    digitalWrite(switchGndPins[row], LOW);
    digitalWrite(ledGndPins[row], LOW);
    int lastColor = 0;
    for (int column = 0; column < 4; column++) {
      int cell = row * 4 + column;
      int desiredColor = buttonColors[cell];

      if (desiredColor > 0) {
        analogWrite(redPins[column], redPalette[desiredColor]);
        analogWrite(greenPins[column], greenPalette[desiredColor]);
        analogWrite(bluePins[column], bluePalette[desiredColor]);
        
//        for (int i = 0; i < stripLength; i++) {
//          strip.setPixelColor((stripPixels[cell] * stripLength) + i, stripPalette[desiredColor]);
//        }
//        strip.show();
    
        delayMicroseconds(100);
    
        analogWrite(redPins[column], 0);
        analogWrite(greenPins[column], 0);
        analogWrite(bluePins[column], 0);

//        for (int i = 0; i < stripLength; i++) {
//          strip.setPixelColor((stripPixels[cell] * stripLength) + i, stripBlack);
//        }
//        strip.show();
      }
      
      if (digitalRead(switchPins[column]) == 0) {
        if ((now - buttonTimes[cell]) > 100) {
          int desiredColor = (buttonColors[cell] + 1) % numColors;
          buttonColors[cell] = desiredColor;
          for (int i = 0; i < stripLength; i++) {
            strip.setPixelColor((stripPixels[cell] * stripLength) + i, stripPalette[desiredColor]);
          }
          strip.show();
        }
        buttonTimes[cell] = now;
      }
    }
    digitalWrite(ledGndPins[row], HIGH);
    digitalWrite(switchGndPins[row], HIGH);
  }  
}

