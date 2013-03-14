#include "turn_control_state.h"

#define DESIRED_DIST 0.25 // dist from the wall I want
#define ACCEPTABLE_PATH_ERROR 0.05 // half the width of the acceptable path to be on
#define ROC_IN_ERROR 0.3 // ROC of distance to turn to if out of path

#define ROC_KP 10.0// degress per m/s

#define SPEED 1.0 // m/s

void turn_control_init(TurnControlState *state) {
  
}

double turn_control_cycle(TurnControlState *state, double cur_dist, double cur_roc, double cur_time) {
  double angle = atan(cur_roc/SPEED);
  double x = cur_dist * cos(angle);
  double x_roc = cur_roc * cos(angle);
  
  double pos_error = x - DESIRED_DIST;
  double desired_roc = 0.0;
  
  
  if (abs(pos_error) > ACCEPTABLE_PATH_ERROR) {
    if (pos_error > 0) {
      desired_roc = -ROC_IN_ERROR;
    } else {
      desired_roc = ROC_IN_ERROR;
    }
  }
  
  double output = 0.0;
  
  // just P control for now
  double roc_error = x_roc - desired_roc;
  output += ROC_KP * roc_error;
  
  Serial.print("X: ");
  Serial.print(x);
  Serial.print(" X/t: ");
  Serial.println(x_roc);
  
  return output;
}
