/*  

  Bill Recognition - IAT 267 Final Project
  Date: 31-03-2023
  By: Max Nielsen

  Pseudocode:
  - Gather any constant color data above 1000 (if more than 10 polls, then cut the stream)
  - Store values in large array
  - Check for which bill was inserted
  - Send bill value over serial to other arduino
  - Other arduino listens for this arduino's message
  - Adds the bill value to the toal amount  
  
*/

// --- LIBRARIES --- //
#include <FastLED.h> //LED Matrix
#include <Wire.h> // To control the matrix
#include<SparkFunISL29125.h> //Colour Sensor

// Color Sensor Variables
SFE_ISL29125 RGB_sensor;

//LED matrix values
#define LED_PIN 2
#define NUM_LEDS 64
CRGB leds [NUM_LEDS];

// Buffer of color values
unsigned int values[150][3];
int index = 0;  // Position in array
int faults = 0; // Non-bill readings
int breakFaults = 10; // Amount of faults until buffer stops reading


void setup() {
  Serial.begin(115200);
  
  if(RGB_sensor.init())  // Initialize color sensor
      Serial.println("Color Sensor Initialization Successful");
      
  FastLED.addLeds<WS2812, LED_PIN, GRB> (leds, NUM_LEDS);  // Initialize LED matrix

  // Setting up LED Matrix
  for(int i=16; i<=48; i++) // Save power by lighting up half of the matrix
    if(i != 0)
    leds[i]=CRGB(255,255,255);
  FastLED.show(); // Turn on the LEDs
}

void loop() {
  checkBills();
}

void checkBills() {
  delay(10);  // Ensure sensor readings have time to change
  
  // Step 1: Read raw value
  unsigned int red= RGB_sensor.readRed();
  unsigned int green= RGB_sensor.readGreen();
  unsigned int blue= RGB_sensor.readBlue();
  
  // Step 2: Check if bill is present
  if(blue > 1000 && green > 1000 && index < 150 && red > 800) {
    values[index][0] = red;  // Add values to buffer
    values[index][1] = green;
    values[index][2] = blue;
    index++;
  }
  else if(index != 0) {  // Store at least one reading
    faults++;	// Bill is not present
  }
  
  // Step 3: Calculate value after buffer is filled or bill is no longer present
  if(faults > 10 || index >= 150) {  
    readValue();  // Calculate value and send to other Arduino
    
    // Reset values
    faults = 0;
    index = 0;
    for(int i = 0; i < 200; i++)
      for(int j = 0; i < 3; i++)
        values[i][j] = 0;
  }
}

// Calculate bill value
void readValue() {
  int redWins = 0;
  int greenWins = 0;
  int blueWins = 0;
  int purpleWins = 0;
  float blueMult = 1.15;
  
  for(int i = 0; i < index; i++) {
    // Testing purposes
    // Serial.print("RED: ");
    // Serial.println(values[i][0]);
    // Serial.print("GREEN: ");
    // Serial.println(values[i][1]);
    // Serial.print("BLUE: ");
    // Serial.println(values[i][2]);
    
    // Convert buffer values into an easy to understand variable 
    int red = values[i][0];
    int green = values[i][1];
    int blue = values[i][2];
    
    // Calculating color occurance  
    if(blue > green)
      if(green < 2700 || blue < 3000)
        purpleWins++;
      else
        blueWins++;
    else if((float)red > (float)blue/blueMult)
      redWins++;
    else if(green > 3100)
      if(blue > 3000)
        blueWins++;
    else
      purpleWins++;
  }
  
  //Testing purposes
  // Serial.println(redWins);
  // Serial.println(blueWins);
  // Serial.println(greenWins);
  // Serial.println(purpleWins);
  
  // Sending bill value to arduino based off of number of wins of each color
  if(purpleWins >= redWins && purpleWins >= blueWins && purpleWins >= greenWins) {  // $10
    Serial.println("2");
    //Serial.println("PURPLE");
  }
  else if(greenWins >= blueWins && greenWins >= redWins && greenWins >= purpleWins) { // $20
    Serial.println("3");
    //Serial.println("GREEN");
  }
  else if(blueWins >= greenWins && blueWins >= redWins && blueWins >= purpleWins) { // $5
    Serial.println("1");
    //Serial.println("BLUE");
  }
  else if(redWins >= blueWins && redWins >= greenWins && redWins >= purpleWins) { // $50
    Serial.println("4");
    //Serial.println("RED");
  }
  
  delay(3000);  // Ensure bill has completely passed through
}
