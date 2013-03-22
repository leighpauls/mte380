#include "Arduino.h"
#include <Servo.h>
#include "ultra_state.h"
#include "turn_control_state.h"

#define DEBUG 1
// The boat's pinout
#define FRONT_ULTRA_TRIGGER 4
#define FRONT_ULTRA_ECHO 5

#define BACK_ULTRA_TRIGGER 6
#define BACK_ULTRA_ECHO 7

#define HEAD_ULTRA_TRIGGER 10
#define HEAD_ULTRA_ECHO 11

#define SWITCH_PIN 12

#define IR_PIN A4

#define STRAIGHTS 1750 //1700FAST LAP//1500
#define TURNS 1750//1700FAST LAP// 1400

#define TURN_DIST 0.9 //1.1 hydroplane
#define TURN_EXIT 1.3//1.4 //
// Sensor objects
struct UltraState front_ultra;
struct UltraState back_ultra;
struct UltraState head_ultra;

// actuator objects
Servo turn_servo;
Servo speed_controller;

// Physical constants of the boat
#define STRAIGHT_ANGLE 88.5 //90.5
#define MAX_TURN_HARDNESS 15//20
#define LEFT_TURN_HARDNESS 21//24 (24 with 0.9 TURN DISTANCE with 1700 is supposed to work decently)

void setup() {
  Serial.begin(9600);
  make_head(&head_ultra, HEAD_ULTRA_TRIGGER, HEAD_ULTRA_ECHO);
  make_ultra(&front_ultra, FRONT_ULTRA_TRIGGER, FRONT_ULTRA_ECHO);
  make_ultra(&back_ultra, BACK_ULTRA_TRIGGER, BACK_ULTRA_ECHO);
  turn_servo.attach(3);
  speed_controller.attach(2);
  pinMode(SWITCH_PIN, INPUT);
  digitalWrite(SWITCH_PIN, HIGH); // internal pull-up
  pinMode(IR_PIN, INPUT);
  digitalWrite(IR_PIN, LOW);
  
  Serial.println("brake for 2");
  Serial.println(SWITCH_PIN);
  speed_controller.writeMicroseconds(1000);
  delay(500);
  
  while (digitalRead(SWITCH_PIN) == HIGH) {}
  delay(10);
  while (digitalRead(SWITCH_PIN) == LOW) {}
  delay(10);
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
  //speed_controller.writeMicroseconds(1600); //Leigh's Final Tune
  speed_controller.writeMicroseconds(STRAIGHTS);
  // speed_controller.writeMicroseconds(2000);
  bool turn = false;
  delay(1000);
  
  turn_servo.write(STRAIGHT_ANGLE+MAX_TURN_HARDNESS);
  delay(500);
  turn_servo.write(STRAIGHT_ANGLE-MAX_TURN_HARDNESS);
  delay(500);
  turn_servo.write(STRAIGHT_ANGLE);
  
  TurnControlState turn_control;
  turn_control_init(&turn_control);
  while (digitalRead(SWITCH_PIN) == LOW) {
    cur_time = micros();
    update_ultra(&front_ultra, cur_time);
    cur_time = micros();
    update_ultra(&back_ultra, cur_time);
    cur_time = micros();
    update_head(&head_ultra, cur_time);
    int turn_angle = STRAIGHT_ANGLE; // - ANGLE_PER_DISTANCE*(0.3 - (double)side_ultra.cur_distance);
    
    //double ir_voltage = readFrontRange(IR_PIN);
    
    //if (ir_voltage > 0.38) {
    double dist_diff = abs(front_ultra.cur_distance - back_ultra.cur_distance);
    if ((head_ultra.cur_distance < TURN_DIST && head_ultra.cur_distance > 0.01) || (turn == true && ((head_ultra.cur_distance < (TURN_EXIT)) || (dist_diff) > 0.03))){
      turn_angle = STRAIGHT_ANGLE - LEFT_TURN_HARDNESS;
      turn = true;
    } 
    else {
      if (turn == true) {
        turn = false;
        //turn_control_clear(&turn_control, front_ultra.cur_distance, back_ultra.cur_distance);
      }
      // find the angle that the controller wants to set the servo to
      double control_hardness = turn_control_cycle(&turn_control, front_ultra.cur_distance, back_ultra.cur_distance, cur_time);
      if (abs(control_hardness) < 4.0) {
        control_hardness = 0.0;
      }
      turn_angle -= max(-MAX_TURN_HARDNESS, min(MAX_TURN_HARDNESS, control_hardness));
    }
    if (turn == true || head_ultra.cur_distance < (TURN_DIST+0.1)) {
      speed_controller.writeMicroseconds(TURNS); 
    }   
    //turn_angle = STRAIGHT_ANGLE - LEFT_TURN_HARDNESS;
    turn_servo.write(turn_angle);
    if (turn==false) {
      speed_controller.writeMicroseconds(STRAIGHTS);
    }

#if DEBUG
      Serial.print(" F: ");
      Serial.print(front_ultra.cur_distance);
      Serial.print(", B: ");
      Serial.print(back_ultra.cur_distance);
      Serial.print(", H: ");
      Serial.println(head_ultra.cur_distance);
#endif
    delayMicroseconds(20);
    
    //delay(0.5);
  }
}
