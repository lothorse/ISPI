#include <Adafruit_NeoPixel.h>


/*Last updated July 30th
 Pins 0,1, 8 and possibly 4 are borked. 
 July 29th 2021
 - pin numbers match soldered hardware
 - LED numbers reduced to 24
 - set knob range : 1023 - about 60
 TODO: Fix calibration to use negative numbers

 Dec. 19th
-Switched to RGB LED strip

July 18th 15:13, Leaf Thorseth
-LEDs now turns off at start of loop

July 11th
-All pin numbers finalized
-Values for knob must be tested and set
-Button bumped

July 5th 13:33, Leaf Thorseth
Patch notes:
  -implemented learn mode, with LED indicators
  -push button now ends calibration


March 26th 23:29, Leaf Thorseth
Patch notes:
-Sett everything up for 24 sensors and 12 channels.
-Implimented back checking so we only send a midi signal if the value has changed.
-Each sensor is now checked 25 times per second. */

int highLowValues[24][3] = {{0, 1023, 0},{0, 1023, 0},{0, 1023, 0},{0, 1023, 0},{0, 1023, 0},{0, 1023, 0},{0, 1023, 0},{0, 1023, 0},{0, 1023, 0},{0, 1023, 0},{0, 1023, 0},{0, 1023, 0},{0, 1023, 0},{0, 1023, 0},{0, 1023, 0},{0, 1023, 0},{0, 1023, 0},{0, 1023, 0},{0, 1023, 0},{0, 1023, 0},{0, 1023, 0},{0, 1023, 0},{0, 1023, 0},{0, 1023, 0}};
//Used for individually calibrating the sensors, last value in each bracket is used for holding the previus value of the sensor to avoid sending unnecaserry midi.

//this is a numbering comment     1       2       3       4       5       6       7       8       9       10       11       12       13      14      15      16      17      18      19      20      21       22       23       24
int channelAndNumber[24][2] = {{1, 10},{2, 11},{3, 12},{4, 13},{5, 14},{6, 15},{7, 16},{8, 17},{9, 18},{10, 19},{11, 20},{12, 21}, {1, 22},{2, 23},{3, 24},{4, 25},{5, 26},{6, 27},{7, 28},{8, 29},{9, 30},{10, 31},{11, 32},{12, 33}};
//Channels go from 1 to 12 and then repeat, cc numbers go from 10 to 33. I decided to put these values in an array instead of using a loop, so as to make it as user friendly as possible, and easy to change individually.
//Each bracket of two numbers corresponds to he analogue pin in the corresponding spot in the array below. Arrays are indexed from 0 and up. For instance, to get the control number for the first sensor you would write
//channelAndNumber[0][1], whereas the channel for the last sensor is found in channelAndNumber[23][0]

//put your 24 analogue pin names here.
//this is a numbering comment  1    2    3    4    5    6    7    8    9   10    11  12  13  14  15  16  17  18  19  20  21   22   23   24
const int analoguePins[24] = {A23, A24, A18, A19, A14, A15, A16, A17, A20, A21, A22, A0, A1, A2, A3, A4, A5, A6, A7, A8, A9, A10, A11, A25};

//const int LEDPins[21] = {0, 43, 30, 29, 28, 27, 26, 25, 24, 44, 12, 11, 10, 9, 8, 7, 6, 5, 4, 3, 2};

const int buttonPin = 2;
const int switchPin = 1;
const int knobPin = A12;
const int LED_PIN = 3;
const int LED_COUNT = 24;

Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

int buttonState = LOW; //digital (HIGH/LOW) value of button (pressed/unpressed).
int switchState = LOW; //digital (HIGH/LOW) value of switch (on/off).
int sensorVal; //Integer to keep track of the current relevant sensor value.

int knobHighLow[2] = {65, 1023};
int knobValue;

uint16_t paleBlueHue = 32768;
uint32_t green = strip.Color(0, 255, 0);
uint32_t yellow = strip.Color(255, 255, 0);
uint32_t purple = strip.Color(99, 3, 48);
uint32_t paleBlue = strip.gamma32(strip.ColorHSV(paleBlueHue, 255, 255));
uint32_t speakerColours[24];

void setup() {
  for(int i=0; i < 24; i++){
    speakerColours[i] = paleBlue;
  }

  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.clear();
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255)

  pinMode(buttonPin, INPUT);
  pinMode(switchPin, INPUT);

  goodMorning();

  while(buttonState != HIGH){
  //Calibrate every sensor until you press the button
    //Serial.print("in first part of calibration");
    for(int i = 0; i < 24; i++){ //first calibrate all max values
      sensorVal = analogRead(analoguePins[i]);
      if(sensorVal > highLowValues[i][0]){
        highLowValues[i][0] = sensorVal;
      }
    }
    for(int i = 0; i < 24; i++){ //then calibrate all min values
      sensorVal = analogRead(analoguePins[i]);
      if(sensorVal < highLowValues[i][1]){
        highLowValues[i][1] = sensorVal;
      }
    }
    buttonState = digitalRead(buttonPin);
  }
  while(buttonState == HIGH){ //wait til the button is released
        delay(5);
        buttonState = digitalRead(buttonPin); //actually check the value
  }
}

void loop() {
  //turn off all LEDs
  strip.clear();
  strip.show();

  switchState = digitalRead(switchPin);
  if(switchState == LOW){
    //Serial.print("switch = low");
    learnMode();
  }
  else if(switchState == HIGH){
    //Serial.print("switch = high");
    listenMode();
  }
}

int outputValue(int sensorNum){ //function that takes in an index and returns the reading of the corresponding sensor/analogue pin.
  int sensorHigh = highLowValues[sensorNum][0];
  int sensorLow = highLowValues[sensorNum][1];
  int knobValue = analogRead(knobPin);
  int knob_Normalised = map(knobValue, knobHighLow[1], knobHighLow[0], 0, 100);
  int lowRange = -110+knob_Normalised; //the lowest value you want to adjust to
  int highRange = 140+knob_Normalised; //the highest value you want to adjust to (determines sensitivity)
  sensorVal = analogRead(analoguePins[sensorNum]);
  int brightness = map(sensorVal, sensorHigh, sensorLow, lowRange, highRange);
  int output;
  if(brightness < 0){
    output = 0;
  }
  else if(brightness > 127){//sett the max value
    output = 127;
  }
  else{
    output = brightness;
  }
  speakerColours[sensorNum] = strip.gamma32(strip.ColorHSV(paleBlueHue - paleBlueHue*output/127, 255, 255));
  return output;
}

void learnMode(){ //activated when switch is "off", sends a near constant stream of midi on the active channel/ccnumber
  int activeControlNumber = 0;
  int value = 1;
  boolean rising = true;
  boolean active = true;
  strip.setPixelColor(activeControlNumber, green);
  strip.show();

  while(switchState == LOW){ //keep running learn until the switch is flipped
    buttonState = digitalRead(buttonPin);
    switchState = digitalRead(switchPin);

    if(buttonState == HIGH){ //check button, switch between active and non active modes

      if(active){ //go to inactive mode, turn of LED
        if(activeControlNumber < 24){
          strip.clear();
          strip.show();
        }
        active = false;
      }
      else{ //reset value, moove on to the next channel/ccnumber
        value = 1;
        activeControlNumber ++;
        if(activeControlNumber >= 25){
          activeControlNumber = 0;
        }
        if(activeControlNumber < 24){
          strip.setPixelColor(activeControlNumber, green);
          strip.show();
        }
        active = true;
      }
      while(buttonState == HIGH){ //wait til the button is released
        delay(5);
        buttonState = digitalRead(buttonPin); //actually check the value
      }
    }

    if(active){
      usbMIDI.sendControlChange(channelAndNumber[activeControlNumber][1], value, channelAndNumber[activeControlNumber][0]); //send a midi signal on the relevant channel/ccnumber
      if(rising){
        value ++;
        if(value == 128){
          rising = false;
          value = 126;
        }
      }
      else{
        value --;
        if(value == 0){
          rising = true;
          value = 2;
        }
      }
      delay(20);
    }
  }
}

void listenMode(){ //activated when switch is "on"
  //reads all the sensors in order, and sends a up to 50 messages on each channel per second, if the value has changed.
  strip.setBrightness(25);
  for(int i=0; i<LED_COUNT; i++) { // For each pixel...
    strip.setPixelColor(i, speakerColours[i]);
  }
  strip.show();   // Send the updated pixel colors to the hardware.
  for(int i = 0; i < 24; i++){

    if(i == 11){ //once we've potentially sent once on each cannel, take a 20ms break.
      delay(20);
    }

    switchState = digitalRead(switchPin);
    if(switchState != HIGH){
      strip.clear();
      strip.setBrightness(50);
      strip.show();
      learnMode();
     }

    int value = outputValue(i); //read current value

    if(value != highLowValues[i][2]){ //Check if the value has changed, if it has, update the stored value and send a midi signal.
      highLowValues[i][2] = value;
      usbMIDI.sendControlChange(channelAndNumber[i][1], value, channelAndNumber[i][0]); //first argument is the cc number, last argument is the channel.
    }
  }

  while (usbMIDI.read()) {
    // ignore incoming messages
  }
  delay(20); //wait 20ms again, once we've been through all 12 channels a second time.
  listenMode();

}

void goodMorning(){
    for(int i=0; i<LED_COUNT; i++) { // For each pixel...
      strip.setPixelColor(i, yellow);
      strip.show();   // Send the updated pixel colors to the hardware.
      delay(80); // Pause before next pass through loop
    }
    strip.clear();
    strip.show();
    delay(500);
    for(int j=0; j<2; j++){
      for(int i=0; i<LED_COUNT; i++) { // For each pixel...
        strip.setPixelColor(i, yellow);
      }
      strip.show();   // Send the updated pixel colors to the hardware.
      delay(500); // Pause before next pass through loop
      strip.clear();
      strip.show();
      delay(500);
    }
}
