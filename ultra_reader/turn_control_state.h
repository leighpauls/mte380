#pragma once

#include "Arduino.h"

struct TurnControlState {
  unsigned long last_time_us;
  double last_angle_error_deg;
  
  double integral;
  
  double dist_diff_low_pass;
};
