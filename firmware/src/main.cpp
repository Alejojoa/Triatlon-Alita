#include <Adafruit_SSD1306.h> 
#include <Adafruit_GFX.h>
#include <U8g2_for_Adafruit_GFX.h>
#include <CD74HC4067.h>
#include <motor.h>
#include <bitmaps_triatlon.h>
#include <progress_bars.h>
#include <multiplexedQTR.h>
#include "BluetoothSerial.h"

/* Global section
--------------------------------------------------------------------------*/

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

/* End of global section
--------------------------------------------------------------------------*/
/* Menu section */

// Define display size in pixels
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

// Define display buttons 
#define PIN_SELECT 18 //18
#define PIN_DOWN 5 //5

#define SPRINTER_SCREEN_MARGIN_X 35
#define SPRINTER_SCREEN_MARGIN_Y 0
#define SPRINTER_SCREEN_WIDTH 60
#define SPRINTER_SCREEN_HEIGHT 64

#define CLEANER_SCREEN_MARGIN_X 0
#define CLEANER_SCREEN_MARGIN_Y 0
#define CLEANER_SCREEN_WIDTH 128
#define CLEANER_SCREEN_HEIGHT 64



#define FIRST_SAFETY_TIMEOUT 3000
#define SECOND_SAFETY_TIMEOUT 2000

#define NUM_MODALITIES 3
#define MAX_ITEM_LENGTH 20

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
U8G2_FOR_ADAFRUIT_GFX u8g2_for_adafruit_gfx;

ProgressBar displayPB(display);

// Prints a menu
void DisplayMenu(){
  if (current_screen == selection) {
    display.clearDisplay();

    // Prints previous item name
    u8g2_for_adafruit_gfx.setFontMode(1);
    u8g2_for_adafruit_gfx.setFontDirection(0);
    u8g2_for_adafruit_gfx.setForegroundColor(WHITE);
    u8g2_for_adafruit_gfx.setFont(u8g2_font_7x14_mf);
    u8g2_for_adafruit_gfx.setCursor(25, 15);
    u8g2_for_adafruit_gfx.print(F(menu_items [previous]));

    // Prints selected item name
    u8g2_for_adafruit_gfx.setFontMode(1);
    u8g2_for_adafruit_gfx.setFontDirection(0);
    u8g2_for_adafruit_gfx.setForegroundColor(WHITE);
    u8g2_for_adafruit_gfx.setFont(u8g2_font_7x14B_mf);
    u8g2_for_adafruit_gfx.setCursor(30, 37);
    u8g2_for_adafruit_gfx.print(F(menu_items [selected]));
  
    // Prints next item name
    u8g2_for_adafruit_gfx.setFontMode(1);
    u8g2_for_adafruit_gfx.setFontDirection(0);
    u8g2_for_adafruit_gfx.setForegroundColor(WHITE);
    u8g2_for_adafruit_gfx.setFont(u8g2_font_7x14_mf);
    u8g2_for_adafruit_gfx.setCursor(25, 59);
    u8g2_for_adafruit_gfx.print(F(menu_items [next]));
    
    // Prints selection frame
    display.drawXBitmap ( 0, 22, epd_bitmap_selected_background, 128, 20, WHITE);
    
    // Prints previous item icon
    display.drawXBitmap ( 4, 2, bitmap_icons[previous], 16, 16, WHITE);

    // Prints selected item icon
    display.drawXBitmap ( 4, 24, bitmap_icons[selected], 16, 16, WHITE);

    // Prints next item icon
    display.drawXBitmap ( 4, 46, bitmap_icons[next], 16, 16, WHITE);

    display.display();
  } 
  else if (current_screen == modality && selected == sprinter) {
    displayPB.load(SPRINTER_SCREEN_MARGIN_X, SPRINTER_SCREEN_MARGIN_Y, SPRINTER_SCREEN_WIDTH, SPRINTER_SCREEN_HEIGHT, FIRST_SAFETY_TIMEOUT);
    displayPB.unload(SPRINTER_SCREEN_MARGIN_X, SPRINTER_SCREEN_MARGIN_Y, SPRINTER_SCREEN_WIDTH, SPRINTER_SCREEN_HEIGHT, SECOND_SAFETY_TIMEOUT);

    current_screen = flags;
  }
  else if (current_screen == modality && selected == areaCleaner) {
    displayPB.load(CLEANER_SCREEN_MARGIN_X, CLEANER_SCREEN_MARGIN_Y, CLEANER_SCREEN_WIDTH, CLEANER_SCREEN_HEIGHT, FIRST_SAFETY_TIMEOUT);   
    displayPB.unload(CLEANER_SCREEN_MARGIN_X, CLEANER_SCREEN_MARGIN_Y, CLEANER_SCREEN_WIDTH, CLEANER_SCREEN_HEIGHT, SECOND_SAFETY_TIMEOUT);

    current_screen = flags;
  }
  else if (current_screen == flags){
    display.clearDisplay();
    display.drawXBitmap( 0, 0, epd_bitmap_flag, 128, 64, WHITE);
    display.display();
  }
  else if (current_screen == modality && selected == sumo){
    /*if (!Ps3.isConnected()) {
      display.clearDisplay();
      display.drawXBitmap( 0, 0, bitmap_screens[selected], 128, 64, WHITE);
      display.display();

      delay(250);

      display.clearDisplay();
      display.display();

      delay(250);
    } else {*/
      display.clearDisplay();
      display.drawXBitmap( 0, 0, bitmap_screens[selected], 128, 64, WHITE);
      display.display();
    //}
  }
  else if (current_screen == modality && selected == sprinter){
    display.clearDisplay();
    display.drawXBitmap( 0, 0, bitmap_screens[selected], 128, 64, WHITE);
    display.display();
  }

  UpdateScreenStatus();

  if (current_screen == selection){  
    motors.StayStill();
  }
}

/* End of menu section
--------------------------------------------------------------------------*/
/* Sprinter section */

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

int proportional;
int derivative;
int integral;
int lastError;

int maxSpeed = 255;
int minSpeed = 130;
int speed = 255;

// PID const
float kp = 0.087;
float ki = 0;
float kd = 0.4;
float pid;
float pidRight;
float pidLeft;

#define BRAKE_TIMEOUT 100
#define BRAKE_SPEED 65

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

  pid = (proportional * kp) + (integral * ki) + (derivative * kd); // PID aftermath
    
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

/* End of sprinter section
--------------------------------------------------------------------------*/
/* SerialBT section */

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

/* End of SerialBT section
--------------------------------------------------------------------------*/
/* Area cleaner section */

/* End of area cleaner section
--------------------------------------------------------------------------*/
/* Sumo section */

/*void ProcessGamepad() {
  // Variables to store the input of each stick
  int rightTriggerValue = (Ps3.event.analog_changed.button.r2);  //Left stick  - y axis - forward/backward car movement
  int leftTriggerValue = (Ps3.event.analog_changed.button.l2);  //Right stick - x axis - left/right car movement

  // Mapping of each stick input to 8bit 
  int forwardSpeed = map(rightTriggerValue, 0, 127, 0, 255);
  int backwardsSpeed = map(leftTriggerValue, 0, 127, 0, 255);

  if (rightTriggerValue > 0) {
    motorRight.MoveForward(forwardSpeed);
    motorLeft.MoveForward(forwardSpeed);
  }
  else if (leftTriggerValue > 0) {
    motorRight.MoveBackwards(backwardsSpeed);
    motorLeft.MoveBackwards(backwardsSpeed);
  }
  else {
    motorRight.StayStill();
    motorLeft.StayStill();
  }
}

void StartSumoModality() {
  if(!Ps3.isConnected()) {Ps3.begin("00:00:00:00:00:04");} 
  else if (Ps3.isConnected()) {ProcessGamepad();}
}*/

/* End of sumo section
--------------------------------------------------------------------------*/
/* Triggers section*/

void StartModalityTriggers() {
  // Calibration trigger
  if (current_screen == calibration){StartSprinterCalibration();}

  // Sumo trigger
  if (current_screen == modality && selected == sumo){/*StartSumoModality();*/}

  // Area cleaner trigger
  if (current_screen == flags && selected == areaCleaner){}

  // Sprinter trigger
  else if (current_screen == flags && selected == sprinter){
    StartSprinterModality();
    
    if (debug) {
      SerialBT.begin("Alita");
      StartTelemetry();
    }
  }
}

/* End of triggers section
--------------------------------------------------------------------------*/
/* Setup and loop section */

#define PIN_LED 23

void setup(){    
  SerialBT.begin("Alita");

  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, HIGH);

  // Begin display connection
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {    
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);  
  }

  u8g2_for_adafruit_gfx.begin(display); // Begins u8g2 for gfx library

  pinMode(PIN_SELECT, INPUT_PULLUP);
  pinMode(PIN_DOWN, INPUT_PULLUP);

  
  //pinMode(signal_input, INPUT);

  // Displays team logo
  display.clearDisplay();
  display.drawBitmap( 0, 0, bitmap_alita, 128, 64, WHITE); // Prints teams logo
  display.display();

  delay(3000);
}

void loop() {
  DisplayMenu();
  StartModalityTriggers();           
} 