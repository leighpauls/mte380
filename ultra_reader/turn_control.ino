#include "turn_control_state.h"

#define DESIRED_DIST 0.25 // meters - dist from the wall I want
#define DEGREES_PER_METER  (3.0 / 0.1) // how hard to turn back to center

#define ANGLE_KP 1.5 // servo degress per boat direction degrees
#define ANGLE_KD 0.1 // server degrees per boat direction degrees per second
#define SENSOR_SPREAD 0.1 // meters - distance between sensors

void turn_control_init(TurnControlState *state) {
  state->last_time_us = 0;
  state->last_angle_error_deg = 0.0;
}

double turn_control_cycle(TurnControlState *state, double front_dist, double back_dist, double cur_time_us) {
  double dist_diff = front_dist - back_dist;
  if (abs(dist_diff) >= SENSOR_SPREAD) {
    dist_diff = SENSOR_SPREAD * dist_diff / abs(dist_diff);
  }
  double angle_deg = asin(dist_diff/SENSOR_SPREAD) * 180.0 / PI;
  
  double x = back_dist;
  
  double pos_error = x - DESIRED_DIST;
  // double desired_angle_deg = -pos_error * DEGREES_PER_METER;
  double desired_angle_deg = 0.0;
  
  double output = 0.0;
  
  // P control
  double angle_error_deg = angle_deg - desired_angle_deg;
  output -= ANGLE_KP * angle_error_deg;
  
  // D control
  if (state->last_time_us != 0) {
    double angle_error_roc = (angle_error_deg - state->last_angle_error_deg) / (0.000001 * (double)(cur_time_us - state->last_time_us));
    output -= ANGLE_KD * angle_error_roc;
  }
  state->last_angle_error_deg = angle_error_deg;
  state->last_time_us = cur_time_us;
  
  Serial.print("X: ");
  Serial.print(x);
  Serial.print(" angle: ");
  Serial.println(angle_deg);
  
  return output;
}
