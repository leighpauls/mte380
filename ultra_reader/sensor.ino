#include "ultra_state.h"

// Speed of sound over a 2 way round-trip
#define METERS_PER_US (340.0 / (2 * 1000.0 * 1000.0))
#define PULSE_WIDTH_US (5*1000)
#define SENSOR_BASE_TIME_US 5882

#define MAX_DISTANCE 10.0
#define MAX_ROUND_TRIP_TIME (unsigned int)(MAX_DISTANCE / METERS_PER_US)


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
