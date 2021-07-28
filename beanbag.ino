#include <PS2X_lib.h>
#include <Servo.h>

// uncomment the drive style you want
#define TANK_DRIVE
//#define ARCADE_DRIVE
//#define CHEZY_DRIVE

/******************************************************************
 * set pins connected to PS2 controller:
 * replace pin numbers by the ones you use
 ******************************************************************/
#define PS2_DAT        2  
#define PS2_CMD        3
#define PS2_SEL        4
#define PS2_CLK        5

/******************************************************************
 * select modes of PS2 controller:
 *   - pressures = analog reading of push-butttons 
 *   - rumble    = motor rumbling
 * uncomment 1 of the lines for each mode selection
 ******************************************************************/
#define PS2_PRESSURES   false
#define PS2_RUMBLE      false

/******************************************************************
 * stuff for motor controllers:
 * Assuming motor controllers use standard servo PWM (1ms = full rev, 2ms = full fwd)
 ******************************************************************/

#define LEFT_MTR  10
#define RIGHT_MTR 11

Servo left_mtr;
Servo right_mtr;

PS2X ps2x; // create PS2 Controller Class

//right now, the library does NOT support hot pluggable controllers, meaning 
//you must always either restart your Arduino after you connect the controller, 
//or call config_gamepad(pins) again after connecting the controller.

int error = 0;
byte type = 0;
byte vibrate = 0;

// Reset func 
void (* resetFunc) (void) = 0;

float map_f(int x, int in_min, int in_max, int out_min, int out_max) {
  float rval = (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
  if (rval < out_min)
    return out_min;
  if (rval > out_max)
    return out_max;
  return rval;
}

void setup(){
 
  Serial.begin(115200);
  
  delay(500);  //added delay to give wireless ps2 module some time to startup, before configuring it
   
  //CHANGES for v1.6 HERE!!! **************PAY ATTENTION*************
  
  //setup pins and settings: GamePad(clock, command, attention, data, Pressures?, Rumble?) check for error
  error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, PS2_PRESSURES, PS2_RUMBLE);
  
  if(error == 0)
    Serial.print("Found Controller, configured successful ");
  else if(error == 1)
    Serial.println("No controller found, check wiring, see readme.txt to enable debug. visit www.billporter.info for troubleshooting tips");
  else if(error == 2)
    Serial.println("Controller found but not accepting commands. see readme.txt to enable debug. Visit www.billporter.info for troubleshooting tips");
  else if(error == 3)
    Serial.println("Controller refusing to enter Pressures mode, may not support it. ");
  
  type = ps2x.readType(); 
  switch(type) {
    case 0:
      Serial.println("Unknown Controller type found ");
      break;
    case 1:
      Serial.println("DualShock Controller found ");
      break;
    case 2:
      Serial.println("GuitarHero Controller found ");
      break;
	case 3:
      Serial.println("Wireless Sony DualShock Controller found ");
      break;
   }


  left_mtr.attach(LEFT_MTR);
  right_mtr.attach(RIGHT_MTR);
}

void loop() {
  /* You must Read Gamepad to get new values and set vibration values
     ps2x.read_gamepad(small motor on/off, larger motor strenght from 0-255)
     if you don't enable the rumble, use ps2x.read_gamepad(); with no values
     You should call this at least once a second
   */  
  if(error == 1){ //skip loop if no controller found
    resetFunc();
  }
  else { //DualShock Controller
    ps2x.read_gamepad(false, vibrate); //read controller and set large motor to spin at 'vibrate' speed
    
    if(ps2x.Button(PSB_L2) && ps2x.Button(PSB_R2)) {
      /*Serial.print("Stick Values:");
      Serial.print(ps2x.Analog(PSS_LY), DEC); //Left stick, Y axis. Other options: LX, RY, RX  
      Serial.print(",");
      Serial.print(ps2x.Analog(PSS_LX), DEC); 
      Serial.print(",");
      Serial.print(ps2x.Analog(PSS_RY), DEC); 
      Serial.print(",");
      Serial.println(ps2x.Analog(PSS_RX), DEC); */

      #ifdef TANK_DRIVE
        int left_amt  = ps2x.Analog(PSS_LY);
        int right_amt = ps2x.Analog(PSS_RY);
      #endif

      #ifdef ARCADE_DRIVE
        int fwd_amt  = ps2x.Analog(PSS_LY)
        int str_amt  = ps2x.Analog(PSS_RX);
        int left_amt  = fwd_amt + str_amt;
        int right_amt = fwd_amt - str_amt;
      #endif

      #ifdef CHEZY_DRIVE
        // https://www.chiefdelphi.com/t/west-coast-drive-code/122715/2
      #endif

      int left_us  = map_f(left_amt, 0, 255, 1000, 2000);
      int right_us = map_f(right_amt, 0, 255, 1000, 2000);
      
      left_mtr.writeMicroseconds(left_us);
      right_mtr.writeMicroseconds(right_us);
      
      Serial.print("Commanded: ");
      Serial.print(left_us, DEC);
      Serial.print(",");
      Serial.print(right_us, DEC);
      Serial.println();
    }
  }
  delay(50);  
}
