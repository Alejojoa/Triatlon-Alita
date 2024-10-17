#include <Adafruit_GFX.h>
#include <U8g2_for_Adafruit_GFX.h>
#include <CD74HC4067.h>
#include <motor.h>
#include <bitmaps_triatlon.h>
#include <progress_bars.h>
#include <multiplexedQTR.h>
#include "BluetoothSerial.h"

#define PIN_MR1 26
#define PIN_MR2 27
#define PIN_ML1 16
#define PIN_ML2 17

#define CHANNEL_MR1 0
#define CHANNEL_MR2 1
#define CHANNEL_ML1 2
#define CHANNEL_ML2 3

#define PWM_FREQUENCY 1000
#define PWM_RESOLUTION 8

bool debug = true;

MotorPair motors(PIN_MR1, PIN_MR2, CHANNEL_MR1, CHANNEL_MR2, PIN_ML1, PIN_ML2, 
                 CHANNEL_ML1, CHANNEL_ML2, PWM_FREQUENCY, PWM_RESOLUTION);

BluetoothSerial SerialBT;

multiplexedQTR qtr;

const uint8_t SensorCount = 8;
uint16_t sensorValues[SensorCount];

int position;

// Gets line position
int getPosition() {
  position = qtr.readLineWhite(sensorValues);
  return position;
}

// Calibrates sprinters QTR sensors
void StartSprinterCalibration() {
  qtr.setTypeAnalog();
  qtr.setSensorPins((const uint8_t[]){0, 1, 2, 3, 4, 5, 6, 7}, SensorCount); 

  delay(500);

  // Prints calibration big icon
  display.clearDisplay();
  display.drawXBitmap( 0, 0, bitmap_calibration, 124, 64, WHITE);
  display.display();

  for (uint16_t i = 0; i < 350; i++){
    qtr.calibrate();   
  }

  current_screen = modality;
}

int setPoint = 1750; // Sets line position

int proportional = 0;
int derivative = 0;
int integral = 0;
int lastError = 0;

int maxSpeed = 255;
int minSpeed = 150;
int speed = 255;

// PID const
float kp = 0.087;
float ki = 0;
float kd = 0.4;
float pid;
float pidRight;
float pidLeft;

#define BRAKE_TIMEOUT 66

#define BLACK_POSITION 7000
#define WHITE_THRESHOLD_MIN 3400
#define WHITE_THRESHOLD_MAX 3600

bool brakeCompleted = false;

bool BlackOffRoad() {
  return position == BLACK_POSITION || position == 0;
}

bool WhiteOffRoad() {
  return position > WHITE_THRESHOLD_MIN && position < WHITE_THRESHOLD_MAX;
}

void Brake() {
  int startingTime = millis(); 

  while (millis() - startingTime < BRAKE_TIMEOUT) {
    motors.Brake();
    delay(10);
  }

  brakeCompleted = true;
}

//PID control system code
void StartSprinterModality(){
  position = getPosition();

  proportional = position - setPoint; // Newest error
  integral += proportional; // Integral of the error
  derivative = proportional - lastError; // Derivative of the error

  pid = (proportional * kp) + (derivative * kd); // PID aftermath
    
  lastError = proportional; // Saves last error

  pidRight = speed + pid;
  pidLeft = speed - pid;

  if (pidRight > maxSpeed){pidRight = maxSpeed;} // Defines speed limits for right motor
  if (pidLeft > maxSpeed){pidLeft = maxSpeed;} // Defines speed limits for left motor
  if (!BlackOffRoad() && !WhiteOffRoad()) {brakeCompleted = false;}

  if (!brakeCompleted && (BlackOffRoad() || WhiteOffRoad())) {
    Brake();
  } else if (pidRight <= minSpeed && pidLeft > minSpeed){ // Turns right 
    motors.TurnRight(minSpeed + (minSpeed - pidRight), pidLeft);
  } else if (pidLeft <= minSpeed && pidRight > minSpeed){ // Turns left
    motors.TurnLeft(pidRight, minSpeed + (minSpeed - pidLeft));
  } else {
    motors.MoveForward(pidRight, pidLeft);
  }
}

bool position_and_pid = false;

char message = SerialBT.read();
char buffer[16];

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth in not enabled! Plese run 'make menuconfig' to and enable it
#endif

void DisplayPositionAndPid() {
  SerialBT.print("- position =");
  SerialBT.println(position);

  SerialBT.print("- derivative = ");
  SerialBT.println(derivative);

  SerialBT.print("- pid =");
  SerialBT.println(pid);
}

void FloatToString (float value, char* buffer, int bufferSize) {
  dtostrf(value, 6, 3, buffer);
}

// Prints SerialBT menu
void DisplayMenuBT(){
  for (int i = 0; i < 10; i++) {    
    SerialBT.println("");
  }

  SerialBT.println("Configuracion Actual:");

  SerialBT.print("- KP = ");
  FloatToString(kp, buffer, sizeof(buffer));
  String kpStr = buffer;
  SerialBT.println(kpStr);

  SerialBT.print("- KD = ");
  FloatToString(kd, buffer, sizeof(buffer));
  String kdStr = buffer;
  SerialBT.println(kdStr);
  
  SerialBT.print("- MAXSPEED = ");
  SerialBT.println(maxSpeed);

  SerialBT.print("- MINSPEED = ");
  SerialBT.println(minSpeed);

  SerialBT.print("- SPEED = ");
  SerialBT.println(speed);

  /*SerialBT.println(" (x) KP + 0.01 / (z) KP - 0.01");
  SerialBT.println(" (t) KP + 0.001 / (g) KP - 0.001");

  SerialBT.println(" (p) KD + 0.01 / (n) KD - 0.01");
  SerialBT.println(" (u) KD + 0.001 / (j) KD - 0.001");

  SerialBT.println(" (b) setPoint + 100 / (c) setPoint - 100");

  SerialBT.println(" (q) maxSpeed + 5 / (a) maxSpeed - 5");
  SerialBT.println(" (w) minSpeed + 5 / (s) minSpeed - 5");
  SerialBT.println(" (e) speed + 5 / (d) speed - 5");*/

}

void StartTelemetry(){
  message = SerialBT.read();

  if(position_and_pid) {DisplayPositionAndPid();}

  switch (message) {
    case 'b': {      
      setPoint += 100;
      DisplayMenuBT();
      break;    
    }
    case 'c': {      
      setPoint -= 100;
      DisplayMenuBT();
      break;   
    }
    case 'q': {      
      maxSpeed += 5;
      DisplayMenuBT();
      break;    
    }
    case 'a': {      
      maxSpeed -= 5;
      DisplayMenuBT();
      break;    
    }
    case 'w': {     
      minSpeed += 5;
      DisplayMenuBT();
      break;    
    }
    case 's': {      
      minSpeed -= 5;
      DisplayMenuBT();
      break;   
    }
    case 'e': {
      speed += 5;
      DisplayMenuBT();
      break;
    }
    case 'd': {
      speed -= 5;
      DisplayMenuBT();
      break;
    }
    case 't': {      
      kp += 0.001;
      DisplayMenuBT();
      break;    
    }
    case 'g': {      
      kp -= 0.001;
      DisplayMenuBT();
      break;   
    }
    case 'x': {      
      kp += 0.01;
      DisplayMenuBT();
      break;    
    }
    case 'z': {      
      kp -= 0.01;
      DisplayMenuBT();
      break;    
    }
    case 'u': {      
      kd += 0.001;
      DisplayMenuBT();
      break;    
    }
    case 'j': {      
      kd -= 0.001;
      DisplayMenuBT();
      break;    
    }
    case 'p': {      
      kd += 0.01;
      DisplayMenuBT();
      break;    
    }
    case 'n': {      
      kd -= 0.01;
      DisplayMenuBT();
      break;    
    }
    case 'r': {
      position_and_pid = !position_and_pid;
      break;
    }  
  }
}

#define PIN_LED 23

void setup(){    
  SerialBT.begin("Alita");

  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, HIGH);
}

void loop() {
  StartSprinterCalibration();
  StartSprinterModality();
} 
