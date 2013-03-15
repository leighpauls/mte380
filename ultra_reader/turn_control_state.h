#pragma once

#include "Arduino.h"

struct TurnControlState {
  unsigned long last_time_us;
  double last_angle_error_deg;
};
