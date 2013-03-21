#include "turn_control_state.h"

#define DESIRED_DIST 0.35 // meters - dist from the wall I want
#define DEGREES_PER_METER  10//(0.5 / 0.1) // boat degrees per position meter - how hard to turn back to center

#define ANGLE_KP 1.0 // servo degress per boat direction degrees
#define ANGLE_KI 0.0
#define ANGLE_KD 0.0 // 0.2 // 0.4 // server degrees per boat direction degrees per second
#define SENSOR_SPREAD 0.1 // meters - distance between sensors
#define ANGLE_BIAS 0.0 // calculated sensor angle which results in a "straight" trajectory

#define LOST_WALL_THRESHOLD 0.8 // meters from the wall where utlra readings are crap
#define LOST_WALL_OUTPUT 0//-10.0 // how hard to turn if the wall is lost

#define DIFF_ALPHA 0.5 // intensity of Low pass on boat angle (sensor difference)

void turn_control_init(TurnControlState *state) {
  state->last_time_us = 0;
  state->last_angle_error_deg = 0.0;
  state->integral = 0.0;
  state->dist_diff_low_pass = 0.0;
}

void turn_control_clear(TurnControlState *state, double front_dist, double back_dist) {  //Clear the value of the accumulated lo pass distance difference and replace it with the new one.
  double dist_diff = front_dist - back_dist;
  state->dist_diff_low_pass = dist_diff;
}

double turn_control_cycle(TurnControlState *state, double front_dist, double back_dist, double cur_time_us) {
  // Low pass the distance difference (input) of the boat angle
  double dist_diff_contrib = front_dist - back_dist;
  state->dist_diff_low_pass = DIFF_ALPHA * dist_diff_contrib + state->dist_diff_low_pass * (1.0 - DIFF_ALPHA);
  
  double dist_diff = state->dist_diff_low_pass;
  
  if (abs(dist_diff) >= (SENSOR_SPREAD * 0.5)) {
    dist_diff = 0.5 * SENSOR_SPREAD * dist_diff / abs(dist_diff);
  }
  double angle_deg = asin(dist_diff/SENSOR_SPREAD) * 180.0 / PI;
  
  if (angle_deg != angle_deg) {
    // nan
    Serial.println("naning");
    angle_deg = 0.0;
  }
  
  double x = back_dist;
  
  double pos_error = x - DESIRED_DIST;
  double desired_angle_deg = -pos_error * DEGREES_PER_METER + ANGLE_BIAS;
  // double desired_angle_deg = ANGLE_BIAS; // uncomment this line for direction control!!!
  
  double output = 0.0;
  
  // P control
  double angle_error_deg = angle_deg - desired_angle_deg;
  output -= ANGLE_KP * angle_error_deg;
  
  // I control
  state->integral += angle_error_deg * (0.000001 * (double)(cur_time_us - state->last_time_us));
  if (abs(state->integral) > (30.0 / ANGLE_KI)) {
    state->integral = (30.0 / ANGLE_KI) * state->integral / abs(state->integral);
  }
  output -= ANGLE_KI * state->integral;
  
  // D control
  if (state->last_time_us != 0) {
    double angle_error_roc = (angle_error_deg - state->last_angle_error_deg) / (0.000001 * (double)(cur_time_us - state->last_time_us));
    output -= ANGLE_KD * angle_error_roc;
  }
  
  state->last_angle_error_deg = angle_error_deg;
  state->last_time_us = cur_time_us;
  
  // override if I've lost the wall completely
  if (front_dist > LOST_WALL_THRESHOLD || back_dist > LOST_WALL_THRESHOLD) {
    Serial.print("f: ");
    Serial.print(front_dist);
    Serial.print(", b:" );
    Serial.print(back_dist);
    Serial.println(", Find Wall");
    return LOST_WALL_OUTPUT;
  }
  
  Serial.print("f: ");
  Serial.print(front_dist);
  Serial.print(", b:" );
  Serial.print(back_dist);
  Serial.print(", diff: ");
  Serial.print(dist_diff);
  Serial.print(", x: ");
  Serial.print(x);
  Serial.print(", a: ");
  Serial.println(angle_deg);
  
  // Serial.print(" int: ");
  // Serial.println(state->integral);
  
  return output;
}
