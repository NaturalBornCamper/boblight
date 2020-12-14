/* t4a_boblight
 * (C) 2014 Hans Luijten, www.tweaking4all.com
 *
 * t4a_boblight is free software and can be distributed and/or modified
 * freely as long as the copyright notice remains in place.
 * Nobody is allowed to charge you for this code.
 * Use of this code is entirely at your own risk.
 */

#include "Adafruit_NeoPixel.h"

// DEFINITIONS

#define STARTCOLOR 0x00FF00  // LED colors at start
#define BLACK      0x000000  // LED color BLACK

#define DATAPIN    6        // Datapin
#define STRIPLEDCOUNT   150       // Number of LEDs in the whole LED strip
#define USEDLEDCOUNT   81       // Number of LEDs used for boblight
#define SHOWDELAY  0       // Delay in micro seconds before showing
#define BAUDRATE   115200    // Serial port speed, 460800 tested with Arduino Uno R3
#define BRIGHTNESS 90        // Max. brightness in %

// Start prefix (Adalight standard, Sending "Ada", then min value, then max value, then checksum (max XOR 0x55))
const char prefix[] = {'A', 'd', 'a', 0, USEDLEDCOUNT-1, (USEDLEDCOUNT-1)^0x55};  // Start prefix

char buffer[sizeof(prefix)]; // Temp buffer for receiving prefix data 

// Init LED strand, WS2811/WS2912 specific
Adafruit_NeoPixel strip = Adafruit_NeoPixel(STRIPLEDCOUNT, DATAPIN, NEO_GRB + NEO_KHZ800);

int state;                   // Define current state
#define STATE_WAITING   1    // - Waiting for prefix
#define STATE_DO_PREFIX 2    // - Processing prefix
#define STATE_DO_DATA   3    // - Receiving LED colors

int readSerial;           // Read Serial data (1)
int currentLED;           // Needed for assigning the color to the right LED

// Converts LED Id from screen capture software to Strip LED id (since there are holes with unused LEDs in the middle)
int bobs[] = {  // 81 LEDs
  0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, // 15
  // Skip 2, then 26:
  17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, // 26
  // Skip 2, then 14:
  45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, // 14
  // Skip 1, then 26
  61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86 // 26
};

void setup()
{
  strip.begin();            // Init LED strand, set all black, then all to startcolor
  turnOffAllLeds();
  
  strip.begin();            // Init LED strand, set all black, then all to startcolor
  strip.setBrightness( (255 / 100) * BRIGHTNESS );

  setAllLEDs(STARTCOLOR, 5);

  Serial.begin(BAUDRATE);   // Init serial speed

  state = STATE_WAITING;    // Initial state: Waiting for prefix
}

void loop()
{
  switch(state)
  {
    case STATE_WAITING:                  // *** Waiting for prefix ***
      if( Serial.available()>0 )
      {
        readSerial = Serial.read();      // Read one character

        if ( readSerial == prefix[0] )   // if this character is 1st prefix char
          { state = STATE_DO_PREFIX; }   // then set state to handle prefix
      }
      break;

    case STATE_DO_PREFIX:                // *** Processing Prefix ***
      if( ((unsigned int)Serial.available()) > sizeof(prefix) - 2 ) 
      {
          Serial.readBytes(buffer, sizeof(prefix) - 1);

          for( unsigned int Counter = 0; Counter < sizeof(prefix) - 1; Counter++) 
          {
            if( buffer[Counter] == prefix[Counter+1] ) 
            {
              state = STATE_DO_DATA;     // Received character is in prefix, continue
              currentLED = 0;            // Set current LED to the first one
            }
            else 
            {
              state = STATE_WAITING;     // Crap, one of the received chars is NOT in the prefix
              break;                     // Exit, to go back to waiting for the prefix
            } // end if buffer
          } // end for Counter
      } // end if Serial
      break;

    case STATE_DO_DATA:                  // *** Process incoming color data ***
      if( Serial.available() > 2 )       // if we received more than 2 chars
      {
        Serial.readBytes( buffer, 3 );   // Abuse buffer to temp store 3 charaters
        strip.setPixelColor( bobs[currentLED++], buffer[0], buffer[1], buffer[2]);  // and assing to LEDs
      }

      if( currentLED > USEDLEDCOUNT )        // Reached the last LED? Display it!
      {
          strip.show();                  // Make colors visible
          delayMicroseconds(SHOWDELAY);  // Wait a few micro seconds

          state = STATE_WAITING;         // Reset to waiting ...
          currentLED = 0;                // and go to LED one

          break;                         // and exit ... and do it all over again
      }
      break; 
  } // switch(state)

} // loop


void turnOffAllLeds(){
  for (int Counter=0; Counter < STRIPLEDCOUNT; Counter++)
    { strip.setPixelColor(Counter, BLACK); }

  strip.show(); 
}


// Sets the color of all LEDs in the strand to 'color'
// If 'wait'>0 then it will show a swipe from start to end
void setAllLEDs(uint32_t color, int wait)
{
  for ( int Counter=0; Counter < USEDLEDCOUNT; Counter++ )      // For each LED
  {
    strip.setPixelColor( bobs[Counter], color );      // .. set the color

    if( wait > 0 )                        // if a wait time was set then
    {
      strip.show();                     // Show the LED color
      delay(wait);                      // and wait before we do the next LED
    } // if wait

  } // for Counter

  strip.show();                         // Show all LEDs
} // setAllLEDs
