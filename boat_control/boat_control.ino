#include <Servo.h>

#define BUTTON 

Servo turn_servo;
Servo speed_controller;

void setup()
{
  turn_servo.attach(3);
  speed_controller.attach(2);
  speed_controller.writeMicroseconds(1000);
  
  /*
  Serial.begin(9600);
  Serial.println("Waiting for line");
  speed_controller.writeMicroseconds(2000);
  while (!Serial.available()) {}
  Serial.read();
  
  Serial.println("waiting for start");
  speed_controller.writeMicroseconds(1000);
  
  while (!Serial.available()) {}
  Serial.read();
  Serial.println("starting");
  */
  delay(1000);
}

#define DEST_CYCLE_LEN_US (10 * 1000)

int fast_remaining = 1000;

void loop()
{
  
  unsigned long cycle_start = micros();
  
  turn_servo.write(100);
  speed_controller.writeMicroseconds(1100);
 
  /*unsigned long cycle_end = micros();
  unsigned long delay_time = DEST_CYCLE_LEN_US - (cycle_end - cycle_start);
  delayMicroseconds(delay_time);
  */
}
