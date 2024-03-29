#include "Arduino.h"
#include <Servo.h>
#include "ultra_state.h"
#include "turn_control_state.h"

// The boat's pinout
#define FRONT_ULTRA_TRIGGER 4
#define FRONT_ULTRA_ECHO 5

#define BACK_ULTRA_TRIGGER 6
#define BACK_ULTRA_ECHO 7

#define SWITCH_PIN 12

#define IR_PIN A4

// Sensor objects
struct UltraState front_ultra;
struct UltraState back_ultra;

// actuator objects
Servo turn_servo;
Servo speed_controller;

// Physical constants of the boat
#define STRAIGHT_ANGLE 90.5
#define MAX_TURN_HARDNESS 20
#define LEFT_TURN_HARDNESS 24

void setup() {
  Serial.begin(9600);
  make_ultra(&front_ultra, FRONT_ULTRA_TRIGGER, FRONT_ULTRA_ECHO);
  make_ultra(&back_ultra, BACK_ULTRA_TRIGGER, BACK_ULTRA_ECHO);
  turn_servo.attach(3);
  speed_controller.attach(2);
  pinMode(SWITCH_PIN, INPUT);
  digitalWrite(SWITCH_PIN, HIGH); // internal pull-up
  pinMode(IR_PIN, INPUT);
  digitalWrite(IR_PIN, LOW);
  
  Serial.println("brake for 2");
  speed_controller.writeMicroseconds(1000);
  
  while (digitalRead(SWITCH_PIN) == HIGH) {}
  delay(5);
  while (digitalRead(SWITCH_PIN) == LOW) {}
  delay(5);
  Serial.println("up for 2");
  speed_controller.writeMicroseconds(2000);
  
  
  while (digitalRead(SWITCH_PIN) == HIGH) {}
  delay(5);
  while (digitalRead(SWITCH_PIN) == LOW) {}
  delay(5);
  
  Serial.println("brake");
  speed_controller.writeMicroseconds(1000);
}

void loop() {
  Serial.println("Anti-ESC fucking delay...");  
  turn_servo.write(STRAIGHT_ANGLE);
  speed_controller.writeMicroseconds(1000);
  
  delay(3000);
  
  Serial.println("Waiting to start...");
  while (digitalRead(SWITCH_PIN) == HIGH) {}
  Serial.println("starting");
  
  unsigned long cur_time = micros();
  
  // speed_controller.writeMicroseconds(1150);
  speed_controller.writeMicroseconds(1600);
  // speed_controller.writeMicroseconds(2000);
  
  delay(1000);
  
  TurnControlState turn_control;
  turn_control_init(&turn_control);

  while (digitalRead(SWITCH_PIN) == LOW) {
    cur_time = micros();
    update_ultra(&front_ultra, cur_time);
    cur_time = micros();
    update_ultra(&back_ultra, cur_time);
    
    int turn_angle = STRAIGHT_ANGLE; // - ANGLE_PER_DISTANCE*(0.3 - (double)side_ultra.cur_distance);
    
    double ir_voltage = readFrontRange(IR_PIN);
    if (ir_voltage > 0.38) {
      // Serial.println("Left");
      turn_angle = STRAIGHT_ANGLE - LEFT_TURN_HARDNESS;
    } else {
      // find the angle that the controller wants to set the servo to
      double control_hardness = turn_control_cycle(&turn_control, front_ultra.cur_distance, back_ultra.cur_distance, cur_time);
      if (abs(control_hardness) < 4.0) {
        control_hardness = 0.0;
      }
      turn_angle -= max(-MAX_TURN_HARDNESS, min(MAX_TURN_HARDNESS, control_hardness));
    }
    turn_servo.write(turn_angle);
 
    // Serial.print("IR: ");
    // Serial.println(ir_voltage);
 
    
    /*Serial.print(" F: ");
    Serial.print(front_ultra.cur_distance);
    Serial.print(", B: ");
    Serial.println(back_ultra.cur_distance);
    */
    delayMicroseconds(20);
  }
}
