// NOT INTENDED TO BE USED AS AN ACTUAL SPEEDOMETER
//
// GPS clock using test code from Adafruit's GPS code.
// Original GPS code here: https://github.com/adafruit/Adafruit_GPS/tree/master/examples/GPS_HardwareSerial_Parsing
// Core LED alphanumeric code here: https://github.com/adafruit/Adafruit_LED_Backpack/tree/master/examples/quadalphanum
//
// Big thanks to Lady Ada from Adafruit and adafruit_support_carter from the Adafruit forums for help getting this going.
//
// Tested and works great with:
// Adafruit Ultimate GPS FeatherWing ------------------------> http://amzn.to/2wpKIdT
// Adafruit 0.54" Quad Alphanumeric FeatherWing Display -----> https://www.adafruit.com/product/3129
// Adafruit Feather M0 Basic Proto - ATSAMD21 Cortex M0 -----> http://amzn.to/2wpuWj5
// Tiny patch uFL GPS Antenna -------------------------------> http://amzn.to/2jqyVsU

#include "Adafruit_LEDBackpack.h"
#include <Adafruit_GPS.h>
#include <Adafruit_GFX.h>
#define GPSSerial Serial1
Adafruit_GPS GPS(&GPSSerial);

// Convert GPS knots to mph
float knot2mph = 1.15078;

// Set a brightness value from 1-15
int displayBrightness = 15;

// This is for the alpha display. This sketch will not work with the 7 segment.
Adafruit_AlphaNum4 alpha4 = Adafruit_AlphaNum4();

// This timer is for the GPS readings.  Don't mess with it.
uint32_t timer = millis();

void setup()  
{
  alpha4.begin(0x70);  // pass in the address

  // 9600 NMEA is the default baud rate for Adafruit MTK GPS's- some use 4800
  GPS.begin(9600);
  
  // uncomment this line to turn on RMC (recommended minimum) and GGA (fix data) including altitude
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  
  // Set the update rate
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);   // 1 Hz update rate
  // For the parsing code to work nicely and have time to sort thru the data, and
  // print it out we don't suggest using anything higher than 1 Hz

  // Set the display brightness
  alpha4.setBrightness(displayBrightness);
  delay(500);
}

// This is our lookup table for the characters 0-9
// This is needed because the alpha display can't read from
// the GPS readings directly
// Plus binary in Arduino sketches looks cool
int getNumber(int numInput){
  if (numInput == 0) {
    return (0b0000110000111111);
  } else if (numInput ==1) {
    return (0b0000000000000110);
  } else if (numInput ==2) {
    return (0b0000000011011011);
  } else if (numInput ==3) {
    return (0b0000000010001111);
  } else if (numInput ==4) {
    return (0b0000000011100110);
  } else if (numInput ==5) {
    return (0b0010000001101001);
  } else if (numInput ==6) {
    return(0b0000000011111101);
  } else if (numInput ==7) {
    return (0b0000000000000111);
  } else if (numInput ==8) {
    return (0b0000000011111111);
  } else if (numInput ==9) {
    return (0b0000000011101111);
  }
}
void loop()                     // run over and over again
{
  // read data from the GPS in the 'main loop'
  char c = GPS.read();
    // if a sentence is received, we can check the checksum, parse it...
  if (GPS.newNMEAreceived()) {
    // a tricky thing here is if we print the NMEA sentence, or data
    // we end up not listening and catching other sentences!
    // so be very wary if using OUTPUT_ALLDATA and trytng to print out data
    if (!GPS.parse(GPS.lastNMEA())) // this also sets the newNMEAreceived() flag to false
      return; // we can fail to parse a sentence in which case we should just wait for another
  }
  // if millis() or timer wraps around, we'll just reset it
  if (timer > millis()) timer = millis();
  if (millis() - timer > 1000) {
    timer = millis(); // reset the timer
    // Clear the LED display
    alpha4.clear();

    //Display the number of satellites in view
    int intSat = (int)GPS.satellites;
    if (intSat < 10) {
      alpha4.writeDigitRaw(1, getNumber(intSat));    
    } else if (intSat > 9) {
      alpha4.writeDigitRaw(0, getNumber(1));
      alpha4.writeDigitRaw(1, getNumber(intSat - 10));  
    }
    // Display Speed in MPH
    float fltMPH = (GPS.speed * knot2mph);
    int intMPH = int(fltMPH);
    if (intMPH < 10) {
      alpha4.writeDigitRaw(2, getNumber(0));
      alpha4.writeDigitRaw(3, getNumber(intMPH));
    } else if ((intMPH > 9) && (intMPH < 20)) {
      alpha4.writeDigitRaw(2, getNumber(1));
      alpha4.writeDigitRaw(3, getNumber(intMPH-10));
    } else if ((intMPH > 19) && (intMPH < 30)) {
      alpha4.writeDigitRaw(2, getNumber(2));
      alpha4.writeDigitRaw(3, getNumber(intMPH-20));
    } else if ((intMPH > 29) && (intMPH < 40)) {
      alpha4.writeDigitRaw(2, getNumber(3));
      alpha4.writeDigitRaw(3, getNumber(intMPH-30));
    } else if ((intMPH > 39) && (intMPH < 50)) {
      alpha4.writeDigitRaw(2, getNumber(4));
      alpha4.writeDigitRaw(3, getNumber(intMPH-40));
    } else if (intMPH > 49) {
      alpha4.writeDigitRaw(2, getNumber(5));
      alpha4.writeDigitRaw(3, getNumber(intMPH-50));
    } else if (intMPH == 0) {
      alpha4.writeDigitRaw(2, getNumber(0));
      alpha4.writeDigitRaw(3, getNumber(0));
    }
    // Write to the LED display
    alpha4.writeDisplay();
  }
}
