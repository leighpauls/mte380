#include "Arduino.h"
#include <Servo.h>

#define FRONT_ULTRA_TRIGGER 4
#define FRONT_ULTRA_ECHO 5
#define SIDE_ULTRA_TRIGGER 6
#define SIDE_ULTRA_ECHO 7

// Speed of sound over a 2 way round-trip
#define METERS_PER_US (340.0 / (2 * 1000.0 * 1000.0))
#define PULSE_WIDTH_US (5*1000)
#define SENSOR_BASE_TIME_US 5882

#define MAX_DISTANCE 10.0
#define MAX_ROUND_TRIP_TIME (unsigned int)(MAX_DISTANCE / METERS_PER_US)

struct UltraState {
  bool expecting_echo; // if there is an echo in round_trip yet
  unsigned long pulse_emit_time_us; // when the sound was emitted
  double cur_distance; // the curent range
  int trigger_pin; // pin number to trigger
  int echo_pin; // pin number called in echo
};

void make_ultra(struct UltraState *ultra, int trigger, int echo) {
  ultra->trigger_pin = trigger;
  ultra->echo_pin = echo;
  
  pinMode(trigger, OUTPUT);
  pinMode(echo, INPUT);
  
  ultra->expecting_echo = false;
  ultra->pulse_emit_time_us = 0;
  ultra->cur_distance = 0;
}

#define ALPHA 0.2

void update_ultra(struct UltraState *ultra, unsigned long cur_time_us) {
  if (ultra->expecting_echo) {
    // There is still a pulse in transit
    unsigned long round_trip_time = cur_time_us - ultra->pulse_emit_time_us;

    if (digitalRead(ultra->echo_pin) == HIGH) {
      ultra->cur_distance = ultra->cur_distance * (1.0 - ALPHA) + ALPHA * (round_trip_time - SENSOR_BASE_TIME_US) * METERS_PER_US;
      ultra->expecting_echo = false;
    } else if (round_trip_time > MAX_ROUND_TRIP_TIME) {
      ultra->cur_distance = MAX_DISTANCE;
      ultra->expecting_echo = false;
    }
  } else if (digitalRead(ultra->echo_pin) == LOW) {
    // there is no pulse left in transit
    ultra->pulse_emit_time_us = cur_time_us;
    digitalWrite(ultra->trigger_pin, HIGH);
    ultra->expecting_echo = true;
  }

  if (cur_time_us - ultra->pulse_emit_time_us > PULSE_WIDTH_US) {
    digitalWrite(ultra->trigger_pin, LOW);
  }
}

struct UltraState front_ultra;
struct UltraState side_ultra;

Servo turn_servo;
Servo speed_controller;

#define STRAIGHT_ANGLE 95
#define ANGLE_PER_DISTANCE (15.0/0.20)
#define PB_PIN 12
#define LEFT_MAX 80
void setup() {
  Serial.begin(9600);
  
  make_ultra(&front_ultra, FRONT_ULTRA_TRIGGER, FRONT_ULTRA_ECHO);
  make_ultra(&side_ultra, SIDE_ULTRA_TRIGGER, SIDE_ULTRA_ECHO);
  
  pinMode(PB_PIN, INPUT);
  
  turn_servo.attach(3);
  turn_servo.write(STRAIGHT_ANGLE);
  speed_controller.attach(2);
  speed_controller.writeMicroseconds(1000);
  delay(1000);
  
  Serial.println("Waiting to start...");
  while (digitalRead(PB_PIN) == HIGH) {}
  Serial.println("starting");
}

void loop() {
  unsigned long cur_time = micros();
  unsigned long next_update_time = cur_time;
  
  unsigned long control_update_time = cur_time;
  speed_controller.writeMicroseconds(1150);
  // Serial.println(MAX_ROUND_TRIP_TIME);
  while (1) {
    cur_time = micros();
    update_ultra(&side_ultra, cur_time);
    update_ultra(&front_ultra, cur_time);
    int turn_angle = STRAIGHT_ANGLE - ANGLE_PER_DISTANCE*(0.3 - (double)side_ultra.cur_distance);
    
    if (front_ultra.cur_distance < 0.20) {
      turn_angle = LEFT_MAX;
    }
    
    if (control_update_time < cur_time) {
       control_update_time += 5 * 1000;
       turn_servo.write(turn_angle);
    }
    
    if (next_update_time < cur_time) {
      // status change
      
      next_update_time += + 0.5 * 1000 * 1000;
      /*Serial.print("A: ");
      Serial.print(turn_angle);
      Serial.print("F: ");
      Serial.print(front_ultra.cur_distance);
      Serial.print(", S: ");
      Serial.println(side_ultra.cur_distance);*/
    }
 
    delayMicroseconds(20);
  }
}
