#pragma once

#include "Arduino.h"

struct UltraState {
  // ping/listening state
  bool expecting_echo; // if there is an echo in round_trip yet
  unsigned long pulse_emit_time_us; // when the sound was emitted
  
  double cur_distance; // the curent range
  
  // rate of change state
  double cur_rate_of_change;
  unsigned long last_reading_time_us;
  
  // sensor pins
  int trigger_pin; // pin number to trigger
  int echo_pin; // pin number called in echo
};
