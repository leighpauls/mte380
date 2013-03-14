#include "Arduino.h"
#include <Servo.h>
#include "ultra_state.h"

// The boat's pinout
#define FRONT_ULTRA_TRIGGER 4
#define FRONT_ULTRA_ECHO 5
#define SIDE_ULTRA_TRIGGER 6
#define SIDE_ULTRA_ECHO 7
#define PB_PIN 12

struct UltraState front_ultra;
struct UltraState side_ultra;

Servo turn_servo;
Servo speed_controller;

// Physical constants of the boat
#define STRAIGHT_ANGLE 95
// #define ANGLE_PER_DISTANCE (15.0/0.20)
#define ANGLE_PER_DISTANCE (0.0)
#define LEFT_MAX 80

void setup() {
  Serial.begin(9600);
  make_ultra(&front_ultra, FRONT_ULTRA_TRIGGER, FRONT_ULTRA_ECHO);
  make_ultra(&side_ultra, SIDE_ULTRA_TRIGGER, SIDE_ULTRA_ECHO);
  turn_servo.attach(3);
  speed_controller.attach(2);
  pinMode(PB_PIN, INPUT);
  digitalWrite(PB_PIN, HIGH); // internal pull-up
}

void loop() {
 
  turn_servo.write(STRAIGHT_ANGLE);
  speed_controller.writeMicroseconds(1000);
  
  delay(3000);
  
  Serial.println("Waiting to start...");
  while (digitalRead(PB_PIN) == HIGH) {}
  Serial.println("starting");
  
  unsigned long cur_time = micros();
  unsigned long next_update_time = cur_time;
  
  unsigned long control_update_time = cur_time;
  speed_controller.writeMicroseconds(1200);
  // speed_controller.writeMicroseconds(2000);
  
  delay(1000);

  while (digitalRead(PB_PIN) == HIGH) {
    cur_time = micros();
    update_ultra(&side_ultra, cur_time);
    update_ultra(&front_ultra, cur_time);
    int turn_angle = STRAIGHT_ANGLE - ANGLE_PER_DISTANCE*(0.3 - (double)side_ultra.cur_distance);
    
    if (front_ultra.cur_distance < 0.40) {
      turn_angle = LEFT_MAX;
    }
    
    if (control_update_time < cur_time) {
       control_update_time += 5 * 1000;
       turn_servo.write(turn_angle);
    }
    
    if (next_update_time < cur_time) {
      // status change
      
      next_update_time += + 0.5 * 1000 * 1000;
      Serial.print("A: ");
      Serial.print(turn_angle);
      Serial.print(" F: ");
      Serial.print(front_ultra.cur_distance);
      Serial.print(", S: ");
      Serial.println(side_ultra.cur_distance);
    }
 
    delayMicroseconds(20);
  }
}
