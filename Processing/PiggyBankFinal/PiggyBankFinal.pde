/*
IAT 267 - Final Project
Team: Animenzz
Project: Smart Piggy Bank

Creation Date: 14-03-2023
Last Edit: 31-03-2023
Created By: Max Nielsen

Description: The Processing aspect of the smart piggy bank.
             Uses serial communication to interface with arduino.
Keeps track of & updates:
  - Plays sound
*/

import processing.serial.*;  // Receiving/transmitting serial
import ddf.minim.*;  // Sound

// Technical Arduino Stuff
String portName = "COM8";  // CHECK IF THIS MATCHES PORT NUMBER IN ARDUINO IDE
Serial port;

Minim minim;  // Sounds manager
AudioPlayer coin, bill, button, win;  // Individual sounds



void setup() {
  port = new Serial(this, portName, 115200);  // Open port to read incoming values
  
  // Initialize sound files
  println("MESSAGE: Initializing SFX...");
  minim = new Minim(this);
  coin = minim.loadFile("Sounds/coins.mp3");
  bill = minim.loadFile("Sounds/bill.mp3");
  button = minim.loadFile("Sounds/button.wav");
  win = minim.loadFile("Sounds/win.wav");
}

void draw() {
  readSerial();
  
  // For testing
  //playSound(coin);
  //delay(2000);
  //playSound(bill);
  //delay(2000);
  //playSound(button);
  //delay(2000);
  //playSound(win);
  //delay(2000);
}

void playSound(AudioPlayer sound) {
  sound.play(0);
}

void readSerial() {
if(port.available() > 0) {
    String rx = port.readStringUntil('\n');
    if(rx != null) {
      rx = rx.trim();
      println(rx);
      if(rx.charAt(0) == 'A') {  // Only check for Arduino's serial outputs
        rx = rx.replaceAll("A", "");  // Removes the prefix to the serial output
        int val = Integer.parseInt(rx);
        
        switch(val) {
          case 1:
            playSound(coin);
            break;
          case 2:
            playSound(bill);
            break;
          case 3:
            playSound(button);
            break;
          case 4:
            playSound(win);
            break;
        }
      }
    }
  }
}
