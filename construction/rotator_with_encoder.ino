/*************************************
voxel_rotator_with_encoder.ino

This program spins the one-axis rotational molder pi/2 rad (1/4 turn) every 45 seconds. Written for an Arduino Uno with Motor Shield v2.3

Typical parameters a user wants to edit marked by **USER**

TODO:
1. An Item

Written by Adam Bilodeau and Dylan Shah. Last edited 2019-011-6 by Dylan Shah.
*************************************/

// Current maintainer: Dylan Shah 

// ***** 1. Import libraries *****
#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"

// ***** 2. Initialize Variables *****
Adafruit_MotorShield AFMS = Adafruit_MotorShield();
Adafruit_DCMotor *molder_motor = AFMS.getMotor(1);
int motor_speed = 75; // PWM value from 0-255. Typically ~75. Tune to allow your motor to move 1/4 turn each cycle. **USER**
unsigned long current_millis = 0; // Time at beginning of current cycle 
unsigned long elapsed = 0; // 
unsigned long millis_to_rest = 1 * 1000; // The motor slows down in <<1 second
unsigned long millis_per_cycle = 45000; // How long between cycles (1/4 turn) [ms]. **USER**
int overshoot = 200; // Approximately how far the system rotates during slowdown (between stop command issued, and actually stopping)

// 2.1 Encoder Variables
volatile long encoder_count = 0; // Use volatile to maintain integrity even though it's modified in an interrupt routine https://www.geeksforgeeks.org/understanding-volatile-qualifier-in-c/
const int ENCODER_PIN = 3; // The Arduino pin that the encoder is plugged into. **USER**

// 100:1 motor https://www.pololu.com/product/2826
int encoder_target = 6533   / 2 / 4 ; // **USER** How many encoder values per 1/4 turn. Even though we specified "CHANGE" below, it appears to only count on rising edge.

// 131:1 motor https://www.pololu.com/product/2827
// https://www.pololu.com/product/4756
//int encoder_target = 8400/ 2 / 4; // **USER** How many encoder values per 1/4 turn. Even though we specified "CHANGE" below, it appears to only count on rising edge.

// ***** 3. Set up Arduino & motor***** 
void setup() {
    // 3.1 Set up serial port
    Serial.begin(2000000);
    Serial.println("Running firmware \"voxel_rotator_with_encoder.ino\"");

    // 3.2. Motor setup
    AFMS.begin();
    molder_motor->setSpeed(motor_speed);
    attachInterrupt(digitalPinToInterrupt(ENCODER_PIN), increment_encoder, CHANGE); // When the encoder rises or falls, increment the encoder count

    // 3.3 Determine motor speed in [counts/(sec)] (Only need to run once, to calibrate/debug when the machine is not going 1/4 turn per cycle)
    // float runtime = 2000;
    // molder_motor->run(FORWARD);
    // delay(runtime);
    // Serial.print("Encoder count after 2 Seconds Rotation = ");
    // Serial.println(encoder_count);
    // Serial.print("Counts per second = ");
    // Serial.println(encoder_count / (runtime/1000) );
    // Serial.print("Target Encoder Count on RISING: ");
    // Serial.println(encoder_target);

    // // Stop rotating
    // molder_motor->run(RELEASE);
    // delay(millis_to_rest);
    // encoder_count = 0;
}

// ***** 4. Loop Forever and Rotate 1/4 turn every 45 seconds ***** 
void loop() {
    current_millis = millis();

    // 4.1. Rotate to encoder_target, or 10 seconds, whichever comes first.
    molder_motor->setSpeed(motor_speed);
    while ((encoder_count < encoder_target - overshoot) && (millis() - current_millis < 10000)) {
      molder_motor->run(FORWARD);
      delay(5);
    }
    // Serial.println(encoder_count);

    // 4.2. Stop rotating
    molder_motor->run(RELEASE); // Stop rotating
    delay(millis_to_rest);
    Serial.print("Encoder count after attempted 1/4 turn: ");
    Serial.println(encoder_count);
    current_millis = millis();

    // 4.3. Wait 45 seconds total
    Serial.print("Waiting until 45000...");
    while (millis() - current_millis < millis_per_cycle){
      delay(1);
    }
    encoder_count = 0;
}


// ***** 5. Increment the variable "encoder_count" every time the Arduino detects a CHANGE on the input pin "ENCODER_PIN" *****
void increment_encoder()
{
  encoder_count++;
  int remainder = encoder_count % 25; // Print to serial every 25 encoder ticks
  if (remainder == 0){    Serial.println(encoder_count);}
}
