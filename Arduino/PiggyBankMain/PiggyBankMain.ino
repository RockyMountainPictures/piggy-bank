#include "SPI.h"
#include "Adafruit_GFX.h"
#include "Adafruit_ILI9341.h"

#include <Servo.h>


#define TFT_CLK 6
#define TFT_MISO 7
#define TFT_MOSI 5
#define TFT_DC 4
#define TFT_CS 2
#define TFT_RST 3

Adafruit_ILI9341 tft= Adafruit_ILI9341(TFT_CS, TFT_DC, TFT_MOSI, TFT_CLK, TFT_RST, TFT_MISO);

//rotate the screen to landscape
#define TFT_ROTATION TFT_ROTATE_180

//define the colours individually
#define IL9341_BLACK 0x0000
#define IL9341_YELLOW 0xFFE0
#define IL9341_WHITE 0xFFFF
#define ILI9341_GREEN 0x07E0
#define ILI9341_CYAN 0x07FF  ///<   0, 255, 255

int valuePos[][2] = {{60, 80}, {0, 160}, {150, 160}};

// Button Variables
const int btnPins[] = {11, 12, 13};
const int numBtns = 3;
int btnValues[numBtns][2];  // 0 = Increase, 1 = Decrease, 2 = Reset
int btnPressed;
int sGoal = 0;
float totalMoney = 0;
bool goalReached = false;

// Coin Variables
const int numCoins = 5;
int sensPins[] = {A4, A3, A2, A1, A0};  // 0.05 to $2
int ledPins[] = {8};
float coinSlots[numCoins][5];  // 0 = raw value of photocell, 1 = brightness on switch, 2 = timer, 3 = value of coin, 4 = ready to accept another coin
int interval = 500;  // In ms

// Servo Variables
int servoPin = 9;
int servoPos = 0;
Servo servo; 

void setup() {
  Serial.begin(115200);
  
  for(int i = 0; i < numCoins; i++) {
    pinMode(sensPins[i], INPUT);
    pinMode(ledPins[i], OUTPUT);
    digitalWrite(ledPins[i], HIGH);
    coinSlots[i][2] = 0;
  }
  
  // Initialize coin values
  coinSlots[0][3] = 0.1;
  coinSlots[1][3] = 0.05;
  coinSlots[2][3] = 0.25;
  coinSlots[3][3] = 1;
  coinSlots[4][3] = 2;
  
  coinSlots[0][1] = 220;
  coinSlots[1][1] = 80;
  coinSlots[2][1] = 130;
  coinSlots[3][1] = 19;
  coinSlots[4][1] = 100;
  
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(ILI9341_BLACK);
  regularDisplay();
  
  delay(500);
  servo.attach(servoPin);
  servoPos = 0;
  servo.write(servoPos);              // tell servo to go to position in variable 'pos
  delay(500);
  servoPos = 90;
  servo.write(servoPos);              // tell servo to go to position in variable 'pos'
  delay(500);
}

void loop() {
  checkButtons();
  checkCoinSlots();
  checkBills();
  checkGoal();
}

void checkGoal() {
  if(totalMoney >= sGoal) {
    servoPos = 0;
    if(goalReached == false && sGoal != 0) {
      showCongratulations(true);
      goalReached = true; 
    }
  }
  else if(sGoal != 0) {
    servoPos = 90;
    if(goalReached == true)
      showCongratulations(false);
    goalReached = false;
  }
  servo.write(servoPos);
  delay(100);
}

void checkBills() {
  if(Serial.available()) {
    String rx = Serial.readStringUntil('\n'); //Read the serial data and store in var
    int value = rx.toInt();
    Serial.println(rx);
    switch(value) {
      case 1:
        totalMoney += 5;
        regularDisplay();
        break;
      case 2:
        totalMoney += 10;
        regularDisplay();
        break;
      case 3:
        totalMoney += 20;
        regularDisplay();
        break;
      case 4:
        totalMoney += 50;
        regularDisplay();
        break;
    }
  }
}

void checkButtons() {
  btnPressed = -1;
  for (int i = 0; i < numBtns; i++) {  // Read values and assign accordingly
    btnValues[i][0] = digitalRead(btnPins[i]);
    if (btnValues[i][0] == 0 && btnValues[i][1] == 1) // Ensure action executes on button release
      btnPressed = i;
    btnValues[i][1] = btnValues[i][0];  // Save values for next cycle
  }

  switch(btnPressed) {
    case 0:
      Serial.println("Adding $5");
      sGoal += 5;
      regularDisplay();
      break;
    case 1:
      Serial.println("Resetting Goal...");
      sGoal = 0;
      totalMoney = 0;
      showCongratulations(false);
      goalReached = false;
      regularDisplay();
      break;
    case 2:
      Serial.println("Subtracting $5");
      sGoal -= 5;
      regularDisplay();
      break;
  }
}

void checkCoinSlots() {
  for(int i = 0; i < numCoins; i++) {  // Check sensor values
    coinSlots[i][0] = analogRead(sensPins[i]);
    
    /*if(i == 3) {
      Serial.print(i);
      Serial.print(": ");
      Serial.println(coinSlots[i][0]);
    }*/
    
    if(coinSlots[i][2] + interval <= millis() && coinSlots[i][0] > coinSlots[i][1])
      coinSlots[i][4] = 1.0;
    
    if(coinSlots[i][0] < coinSlots[i][1] && coinSlots[i][4] == 1.0) {
      Serial.print("Coin accepted: $");
      Serial.println(coinSlots[i][3]);
      totalMoney += coinSlots[i][3];
      regularDisplay();
      coinSlots[i][2] = millis();
      coinSlots[i][4] = 0;
    }
  }
}

void regularDisplay(){
  tftOverwrite();
  tft.setTextColor(ILI9341_CYAN);
  tft.setCursor(10,30);
  tft.setTextSize(3);
  tft.print("Total Money Saved");
  tft.setCursor(60,80);
  tft.setTextSize(5);
  tft.print("$");
  tft.println(totalMoney);

  tft.setCursor(0,150);
  tft.setTextColor(ILI9341_YELLOW);
  tft.setTextSize(1);
  tft.println("Savings Goal:");
  tft.setCursor(0, 160);
  tft.setTextSize(2);
  tft.print("$");
  tft.println(sGoal);

  tft.setCursor(150, 150);
  tft.setTextColor(ILI9341_YELLOW);
  tft.setTextSize(1);
  tft.println("Amount Needed to Reach Goal:");
  tft.setCursor(150, 160);
  tft.setTextSize(2);
  tft.print("$");
  tft.println(sGoal-totalMoney);
  
  tft.setCursor(0, 225);
  tft.setTextColor(ILI9341_WHITE);
  tft.setTextSize(1);
  tft.println("-Goal                 Reset                     +Goal");
}

void showCongratulations(bool show) {
  int x = 20;
  int y = 190;
  if(show) {
    tft.setCursor(x, y);
    tft.setTextColor(ILI9341_GREEN);
    tft.setTextSize(2);
    tft.println("CONGRATS! GOAL REACHED!");
  }
  else
    tft.fillRect(x, y, 290, 25, IL9341_BLACK);
} 

void tftOverwrite() {
  tft.fillRect(valuePos[0][0] + 25, valuePos[0][1], 180, 50, IL9341_BLACK);
  tft.fillRect(valuePos[1][0] + 10, valuePos[1][1], 36, 25, IL9341_BLACK);
  tft.fillRect(valuePos[2][0] + 10, valuePos[2][1], 100, 25, IL9341_BLACK);
}
