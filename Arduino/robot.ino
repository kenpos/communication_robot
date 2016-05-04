#include <Servo.h>

Servo servo,servo1;
int servoPosition = 90;
int servoPosition1 = 90;
int incomingByte = 0;   // for incoming serial data

void setup()
{
  Serial.begin(9600); // // opens serial port, sets data rate to 9600 bps
  
  servo.attach(5); // attaches the servo on pin 5 to the servo object
  servo.write(servoPosition); // set the servo at the mid position
  servo1.attach(6); // attaches the servo on pin 5 to the servo object
  servo1.write(servoPosition); // set the servo at the mid position
}

void loop()
{
  if (Serial.available() > 0) {
    // read the incoming byte:
    incomingByte = Serial.read();
    
    switch(incomingByte)
    {
      // Rotate camera left
      case 'l':
      servoPosition+=2;
      
      if (servoPosition > 180)
      {
        servoPosition = 180;
      }
 servo.write(servoPosition);
      break;
      
      // Rotate camera right
      case 'r':
      servoPosition-=2;
      
      if (servoPosition < 0)
      {
        servoPosition = 0;
      }
 servo.write(servoPosition);
      break;
      
      // Center camera
      case 'c':
      servoPosition = 90;
      servoPosition1 = 90;
       servo.write(servoPosition);
       servo1.write(servoPosition1);
      break;
    
    
      // Servoの上下
      case 'd':
      servoPosition1+=2;
      
      if (servoPosition1 > 180)
      {
        servoPosition1 = 180;
      }
 servo1.write(servoPosition1);
      break;
      
      // 下
      case 'u':
      servoPosition1-=2;
      
      if (servoPosition1 < 0)
      {
        servoPosition1 = 0;
      }
 servo1.write(servoPosition1);
      break;
     
    }
 
  }
}
