#include "turn_control_state.h"

#define DESIRED_DIST 0.25 // meters - dist from the wall I want
#define DEGREES_PER_METER  (15.0 / 0.1) // how hard to turn back to center

#define ANGLE_KP 1.0 // servo degress per boat direction degrees

#define SENSOR_SPREAD 0.1 // meters - distance between sensors

void turn_control_init(TurnControlState *state) {
  
}

double turn_control_cycle(TurnControlState *state, double front_dist, double back_dist, double cur_time) {
  double dist_diff = front_dist - back_dist;
  if (abs(dist_diff) > SENSOR_SPREAD) {
    dist_diff = SENSOR_SPREAD * dist_diff / abs(dist_diff);
  }
  double angle_deg = asin(dist_diff/SENSOR_SPREAD) * 180.0 / PI;
  
  double x = back_dist;
  
  double pos_error = x - DESIRED_DIST;
  double desired_angle_deg = -pos_error * DEGREES_PER_METER;
  
  double output = 0.0;
  
  // just P control for now
  double angle_error_deg = angle_deg - desired_angle_deg;
  output -= ANGLE_KP * angle_error_deg;
  
  // TODO: D control
  
  Serial.print("X: ");
  Serial.print(x);
  Serial.print(" angle: ");
  Serial.println(angle_deg);
  
  return output;
}
