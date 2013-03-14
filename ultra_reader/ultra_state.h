#pragma once

#include "Arduino.h"

struct UltraState {
  double cur_distance; // the curent range
  
  // rate of change state
  double cur_rate_of_change;
  unsigned long last_reading_time_us;
  
  // sensor pins
  int trigger_pin; // pin number to trigger
  int echo_pin; // pin number called in echo
};
