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
   
  ultra->cur_distance = 0;
  
  ultra->cur_rate_of_change = 0;
  ultra->last_reading_time_us = 0;
}

void make_head(struct UltraState *ultra, int trigger, int echo) {
  ultra->trigger_pin = trigger;
  ultra->echo_pin = echo;
  uint8_t EnPwmCmd[4]={0x44,0x02,0xbb,0x01};    
  pinMode(trigger, OUTPUT);
  digitalWrite(trigger,HIGH);
  pinMode(echo, INPUT);
  
  for(int i=0;i<4;i++){
    Serial.write(EnPwmCmd[i]);
  }  
   
  ultra->cur_distance = 0;
  ultra->cur_rate_of_change = 0;
  ultra->last_reading_time_us = 0;
}

#define ALPHA 0.2

void update_ultra(struct UltraState *ultra, unsigned long cur_time_us) {
  digitalWrite(ultra->trigger_pin, HIGH);
  delayMicroseconds(2);
  digitalWrite(ultra->trigger_pin, LOW);
  delayMicroseconds(5);
  
  unsigned long duration_us = pulseIn(ultra->echo_pin, HIGH);
  
  double new_distance = (double)duration_us / 2900.0 / 2.0;
  ultra->cur_rate_of_change = 1000000.0 * (new_distance - ultra->cur_distance) / (double)(cur_time_us - ultra->last_reading_time_us);
  ultra->cur_distance = new_distance;
  ultra->last_reading_time_us = cur_time_us;
}

double readFrontRange(int pin_num) {
  // convert from ADC range to 0V0-5V0 range
  double voltage = (double)(analogRead(pin_num)) * 5.0 / 1023.0;
  return voltage;
}

void update_head(struct UltraState *ultra, unsigned long cur_time_us) {
    
  digitalWrite(ultra->trigger_pin, LOW);
  digitalWrite(ultra->trigger_pin, HIGH);               // reading Pin PWM will output pulses
     
  unsigned long DistanceMeasured=pulseIn(ultra->echo_pin,LOW);
  double new_distance = 0; 
  if(DistanceMeasured==50000){              // the reading is invalid.
    //Serial.print("Invalid");    
  }
   else{
    new_distance=(double)DistanceMeasured / 2900.0 / 2.0;           // every 50us low level stands for 1cm
   }
  //Serial.print("Distance=");
  //Serial.print(new_distance);
  //Serial.println("m");
 
  ultra->cur_rate_of_change = 1000000.0 * (new_distance - ultra->cur_distance) / (double)(cur_time_us - ultra->last_reading_time_us);
  ultra->cur_distance = new_distance;
  ultra->last_reading_time_us = cur_time_us;    
}
