/*
 * Arduino-powered Jack-O-Lantern.
 * Uses a PIR motion sensor to detect trick-or-treaters, triggering music and sound effects.
 * Tested with a Seeeduino Xiao but should be compatible with any Arduino board.
 * Uses the DFPlayer Mini MP3 player for music and WS2812b LEDs for the visuals.
 * 
 * Author: Sean Carolan <scarolan@gmail.com>
 */


#include <WS2812FX.h>
#include "SoftwareSerial.h"
#include "DFRobotDFPlayerMini.h"
#define LED_COUNT 8
#define LED_PIN 8
#define SENSOR_PIN 7
int soundFX = 0;
uint8_t myModes[] = {52, 27, 50, 46, 43, 2, 25, 20, 8};
uint8_t myModeIndex = 0;

// Parameter 1 = number of pixels in strip
// Parameter 2 = Arduino pin number (most are valid)
// Parameter 3 = pixel type flags, add together as needed:
WS2812FX ws2812fx = WS2812FX(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

// Create a software serial connection to DFPlayer Mini
SoftwareSerial mySoftwareSerial(9,10); // RX, TX

// Start MP3 player and do diagnostic check.
DFRobotDFPlayerMini myDFPlayer;
void printDetail(uint8_t type, int value);

void setup()
{
  delay(3000);
  // Open connection to DFPlayer mini
  mySoftwareSerial.begin(9600);
  Serial.begin(115200);
  Serial.println();
  Serial.println(F("DFRobot DFPlayer Mini Demo"));
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));
  
  if (!myDFPlayer.begin(mySoftwareSerial)) {  //Use softwareSerial to communicate with mp3.
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while(true){
      delay(0); // Code to compatible with ESP8266 watch dog.
    }
  }
  Serial.println(F("DFPlayer Mini online."));

  //Get the total number of tracks
  soundFX = myDFPlayer.readFileCounts();
  Serial.print(soundFX);
  Serial.println(" sounds were found on the SD card.");

  //Set volume value. From 0 to 30
  myDFPlayer.volume(30);

  // Initialize the blinkenlights
  Serial.println("Initializing the LEDs.");
  ws2812fx.init();
  ws2812fx.setBrightness(100);
  ws2812fx.setSpeed(500);
  ws2812fx.setMode(FX_MODE_FIRE_FLICKER);
  ws2812fx.start();
}

void playFX() {
  if (soundFX > 0) {
    Serial.print("Playing track: ");
    Serial.println(soundFX);
    myDFPlayer.play(soundFX);
    soundFX--;
  } else {
    soundFX = myDFPlayer.readFileCounts();
  }
}

void playLights() {
  myModeIndex = (myModeIndex + 1) % sizeof(myModes);
  ws2812fx.setMode(myModes[myModeIndex]);
  Serial.print("Mode Name: "); Serial.println(ws2812fx.getModeName(ws2812fx.getMode()));
  Serial.print("Mode Number: "); Serial.println(ws2812fx.getMode());
}

void loop()
{
  static unsigned long timer = millis();
  ws2812fx.service();
  int sensorValue = analogRead(SENSOR_PIN);
  //Serial.println(sensorValue);
  if (sensorValue < 128) {
    //Serial.println("Motion detected.");
    if (millis() - timer > 3000) {
      timer = millis();
      playFX();
      playLights();
    }
    if (myDFPlayer.available()) {
      printDetail(myDFPlayer.readType(), myDFPlayer.read());
    }
  }
  // If we're not doing anything, reset to fire flicker
  if (millis() - timer > 60000) {
    Serial.println("Resetting to fire flicker soft.");
    ws2812fx.setMode(FX_MODE_FIRE_FLICKER);
    timer = millis();
  }
}

// Useful error messages for the DFPlayer mini
void printDetail(uint8_t type, int value){
  switch (type) {
    case TimeOut:
      Serial.println(F("Time Out!"));
      break;
    case WrongStack:
      Serial.println(F("Stack Wrong!"));
      break;
    case DFPlayerCardInserted:
      Serial.println(F("Card Inserted!"));
      break;
    case DFPlayerCardRemoved:
      Serial.println(F("Card Removed!"));
      break;
    case DFPlayerCardOnline:
      Serial.println(F("Card Online!"));
      break;
    case DFPlayerUSBInserted:
      Serial.println("USB Inserted!");
      break;
    case DFPlayerUSBRemoved:
      Serial.println("USB Removed!");
      break;
    case DFPlayerPlayFinished:
      Serial.print(F("Number:"));
      Serial.print(value);
      Serial.println(F(" Play Finished!"));
      break;
    case DFPlayerError:
      Serial.print(F("DFPlayerError:"));
      switch (value) {
        case Busy:
          Serial.println(F("Card not found"));
          break;
        case Sleeping:
          Serial.println(F("Sleeping"));
          break;
        case SerialWrongStack:
          Serial.println(F("Get Wrong Stack"));
          break;
        case CheckSumNotMatch:
          Serial.println(F("Check Sum Not Match"));
          break;
        case FileIndexOut:
          Serial.println(F("File Index Out of Bound"));
          break;
        case FileMismatch:
          Serial.println(F("Cannot Find File"));
          break;
        case Advertise:
          Serial.println(F("In Advertise"));
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}
