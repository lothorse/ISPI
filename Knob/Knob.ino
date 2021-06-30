#include <Adafruit_NeoPixel.h>



//This is just a knob test

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

int knobHighLow[2] = {0, 1023};
int knobValue;

uint32_t green = strip.Color(0, 255, 0);

void setup() {
  
  strip.begin();           // INITIALIZE NeoPixel strip object (REQUIRED)
  strip.show();            // Turn OFF all pixels ASAP
  strip.setBrightness(50); // Set BRIGHTNESS to about 1/5 (max = 255
  Serial.begin(9600);
  pinMode(buttonPin, INPUT);
  pinMode(switchPin, INPUT);
  
  while(buttonState != HIGH){
  //Calibrate every sensor until you press the button
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
  listenMode();
}

int outputValue(int sensorNum){ //function that takes in an index and returns the reading of the corresponding sensor/analogue pin.
  int sensorHigh = highLowValues[sensorNum][0];
  int sensorLow = highLowValues[sensorNum][1];
  int lowRange = 0; //the lowest value you want to adjust to
  int highRange = 255; //the highest value you want to adjust to (determines sensitivity)
  sensorVal = analogRead(analoguePins[sensorNum]);
  int brightness = map(sensorVal, sensorHigh, sensorLow, lowRange, highRange);
  int output;
  if(brightness < 20){ //set the minimum number to be registered as significant. Here, anything below 8 will be ignored as noise.
    output = 0;
  }
  else if(brightness > 127){//sett the max value
    output = 127;
  }
  else{
    output = brightness;
  }
  return output;
}

void listenMode(){ //activated when switch is "on"
  //reads all the sensors in order, and sends a up to 50 messages on each channel per second, if the value has changed.
  for(int i = 0; i < 24; i++){
    
    if(i == 11){ //once we've potentially sent once on each cannel, take a 20ms break.
      delay(20);
    }
    
     
    int value = outputValue(i); //read current value
    
    if(value != highLowValues[i][2]){ //Check if the value has changed, if it has, update the stored value and send a midi signal.
      highLowValues[i][2] = value;
    }
  }
  
  knobValue = analogRead(knobPin);
  Serial.print("Knob value: ");
  Serial.print(knobValue);
  Serial.print("\n");
  delay(20); //wait 20ms again, once we've been through all 12 channels a second time.
  
}
