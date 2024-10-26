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

#define PIN_MR1 16
#define PIN_MR2 17
#define PIN_ML1 27
#define PIN_ML2 26

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
#define PIN_SELECT 18
#define PIN_DOWN 5

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

Adafruit_SH1106 display(OLED_RESET);
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
#define QRE_BLACK              3900
#define SHARP_ATTACK           1000
#define SET_OUT_BACKWARDS_TIME  250 

/*#define LOW_SPEED   100
#define MID_SPEED   150
#define FULL_SPEED  200*/

int LOW_SPEED =  65;
int MID_SPEED =  85;
int FULL_SPEED = 130;

bool offRoad;
//bool object_front;

int sharp_left;
int sharp_right;
int sharp_front_right;
int sharp_front;
int sharp_front_left;
int qre_back;
int qre_left;
int qre_right;

int signal_input;

CD74HC4067 my_mux(4, 25, 33, 32); // s0, s1, s2, s3

#define PIN_SIG 34

void ReadCleanerSensors() { 

  for (int x = 8; x < 15; x++) {
    
    my_mux.channel(x);
  
    signal_input = analogRead(PIN_SIG);

    switch (x) {

      case 8: {
        qre_left = signal_input;
        break;
      }
      case 9: {
        qre_back = signal_input;
        break;
      }
      case 10: {
        qre_right = signal_input;
        break;
      }
      case 11: {
        sharp_right = signal_input;
        break;
      }
      case 12: {
        sharp_front_right= signal_input;
        break;
      }
      case 13: {
        sharp_front = signal_input;
        break;
      }
      case 14: {
        sharp_front_left = signal_input;
        break;
      }
      case 15: {
        sharp_left = signal_input;
        break;
      }
      
    } 
  }
}

void Telemetry() {
  ReadCleanerSensors();

  int datoBT;

  datoBT = SerialBT.read();

  switch (datoBT) {
    case 'A': {
      SerialBT.print ("sharp_left = ");
      SerialBT.println (sharp_left);
      SerialBT.print ("sharp_front_left = ");
      SerialBT.println (sharp_front_left);
      SerialBT.print ("sharp_front = ");
      SerialBT.println (sharp_front);
      SerialBT.print ("sharp_front_right = ");
      SerialBT.println (sharp_front_right);
      SerialBT.print ("sharp_right = ");
      SerialBT.println (sharp_right);
      SerialBT.print ("qre_left = ");
      SerialBT.println (qre_left);
      SerialBT.print ("qre_right = ");
      SerialBT.println (qre_right);
      SerialBT.print ("qre_back = ");
      SerialBT.println (qre_back);
      break;
    }

    case 'sharp': {
      SerialBT.print ("sharp_left = ");
      SerialBT.println (sharp_left);
      SerialBT.print ("sharp_front_left = ");
      SerialBT.println (sharp_front_left);
      SerialBT.print ("sharp_front = ");
      SerialBT.println (sharp_front);
      SerialBT.print ("sharp_front_right = ");
      SerialBT.println (sharp_front_right);
      SerialBT.print ("sharp_right = ");
      break;
    }
    case 'qre': {
      SerialBT.print ("qre_left = ");
      SerialBT.println (qre_left);
      SerialBT.print ("qre_right = ");
      SerialBT.println (qre_right);
      SerialBT.print ("qre_back = ");
      SerialBT.println (qre_back);
      break;
    }
  }
}

void CheckOffRoad() {
  ReadCleanerSensors();

  if (qre_left > QRE_BLACK || qre_right > QRE_BLACK || qre_back > QRE_BLACK) {
    offRoad = true;
  } else {
    offRoad = false;
  }
}


bool out_lastMovment;
int BarzolaTurn = 200;


void Out() { 
  ReadCleanerSensors();

  unsigned long CurrentTime_OUT;
    
  if (qre_left > QRE_BLACK || qre_right > QRE_BLACK) {   
    CurrentTime_OUT = millis();

    /*while (millis() < CurrentTime_OUT + SET_OUT_BACKWARDS_TIME) {
      motors.Brake();
      delay (10);
    }*/

    while (millis() < CurrentTime_OUT + SET_OUT_BACKWARDS_TIME) {
      motors.MoveBackwards (FULL_SPEED , FULL_SPEED);

      if (qre_back > QRE_BLACK) {
        break;
      }
      delay (10);
    }

    while (millis() < CurrentTime_OUT + 100) {
      motors.Brake();
      delay (10);
    }

    while (millis() < CurrentTime_OUT + BarzolaTurn) {
      motors.TurnRight (MID_SPEED , MID_SPEED);
      delay (10);
    }

    out_lastMovment = true;
    CheckOffRoad();
    SerialBT.println ("FRONT");
  }

  if (qre_back > QRE_BLACK) {
    CurrentTime_OUT = millis();

    /*while (millis() < CurrentTime_OUT + SET_OUT_BACKWARDS_TIME) {
      motors.MoveBackwards (MID_SPEED , MID_SPEED);
      delay (10);
    }*/

    if (out_lastMovment) {
      while (millis() < CurrentTime_OUT + 100) {
        motors.MoveForward (MID_SPEED , MID_SPEED);
        delay (10);
      }
    } else if (!out_lastMovment) {
      while (millis() < CurrentTime_OUT + SET_OUT_BACKWARDS_TIME) {
        motors.MoveBackwards (FULL_SPEED , FULL_SPEED);
        delay (10);
      }
    }
  }
}



bool state_found_left;
bool state_found_right;
bool object_back;
long time_found_left;
long time_found_right;


void OutObjectBack() {
  ReadCleanerSensors();
  CheckOffRoad();
  long CurrentTime_ObjectBack;
  int total_found_left;
  int total_found_right;

  if (state_found_left) {
    total_found_left = millis() - (time_found_left + SET_OUT_BACKWARDS_TIME);
  }
  if (state_found_right) {
    total_found_right = millis() - (time_found_right + SET_OUT_BACKWARDS_TIME);
  }

  if (state_found_left && !state_found_right) {
    while (millis() < CurrentTime_ObjectBack + total_found_left) {
      motors.MoveBackwards (MID_SPEED , MID_SPEED);
      delay (10);
    }
  }
  else if (!state_found_left && state_found_right) {
    while (millis() < CurrentTime_ObjectBack + total_found_right) {
      motors.MoveBackwards (MID_SPEED , MID_SPEED);
      delay (10);
    }
  }
  else if (state_found_left && state_found_right) {
    if (total_found_left < total_found_right) {
      while (millis() < CurrentTime_ObjectBack + total_found_left) {
        motors.MoveBackwards (MID_SPEED , MID_SPEED);
        delay (10);
      }
    }
    else if (total_found_left > total_found_right) {
      while (millis() < CurrentTime_ObjectBack + total_found_right) {
        motors.MoveBackwards (MID_SPEED , MID_SPEED);
        delay (10);
      }
    }
  }

  object_back = false;
  state_found_left = false;
  state_found_right = false;
  total_found_left  = 0;
  total_found_right = 0;
  time_found_left  = 0;
  time_found_right = 0;
}

void SearchAllTheTrack() {
  ReadCleanerSensors();
  CheckOffRoad();
  long CurrentTime_SearchAllTheTrack;
  int turn_time = 100;

  for (int i=0 ; i<20 ; i++) {
    CurrentTime_SearchAllTheTrack = millis();

    if (!offRoad) {
      motors.MoveForward (MID_SPEED , MID_SPEED);
    } else {
      motors.TurnRight (LOW_SPEED , MID_SPEED);
    }

    if (sharp_front_left > SHARP_ATTACK || sharp_front > SHARP_ATTACK || sharp_front_right > SHARP_ATTACK) {
      break;
    }
    else if (sharp_right < SHARP_ATTACK) {
      motors.TurnLeft (LOW_SPEED , LOW_SPEED);
      if (sharp_front_left > SHARP_ATTACK || sharp_front > SHARP_ATTACK || sharp_front_right > SHARP_ATTACK) {
        break;
      }
    }


    i = i+1;
  }
}

void SearchObject() {  

  ReadCleanerSensors();  
  CheckOffRoad();

  SerialBT.println ("SEARCH");

  long CurrentTime_SearchObject = millis();
    
  for (int i=0 ; i<10 ; i++) {
    long CurrentTime_Brake = millis() + 200;
    long CurrentTime_TurnRight = millis() + 100;

    while (millis() < CurrentTime_TurnRight) { 
      motors.TurnRight (0 , MID_SPEED);
      ReadCleanerSensors();
    }
    while (millis() < CurrentTime_Brake) {
      motors.Brake(); 
      ReadCleanerSensors();
    }

    //motors.TurnRight (0 , TURN_SPEED);

    if (sharp_front_left > SHARP_ATTACK || sharp_front > SHARP_ATTACK || sharp_front_right > SHARP_ATTACK) {
      break;
    }

    if (offRoad) {
      break;
    }

    i = i+1;

    SerialBT.println (i);
  }

  while (millis() < CurrentTime_SearchObject + 2000) {
    ReadCleanerSensors();
    CheckOffRoad();
    motors.MoveForward (MID_SPEED , MID_SPEED);

    if (sharp_front_left > SHARP_ATTACK || sharp_front > SHARP_ATTACK || sharp_front_right > SHARP_ATTACK) {
      break;
    }

    if (offRoad) {
      break;
    }
    delay (10);
  }

  //SearchAllTheTrack();
  
}


void StartAreaCleanerSection() {
  ReadCleanerSensors();
  CheckOffRoad();

  if (offRoad) {
    Out();
    /*if (object_back) {
      OutObjectBack();
    }*/
  }

  else {
    if (sharp_front_left > SHARP_ATTACK || sharp_front > SHARP_ATTACK || sharp_front_right > SHARP_ATTACK) {
      /*SerialBT.println ("ATTACKING");

      out_lastMovment = false;

      motors.MoveForward (MID_SPEED , MID_SPEED);
      
      ReadCleanerSensors();
      CheckOffRoad();*/

      out_lastMovment = false;

      if (sharp_front_left > sharp_front) {
        motors.TurnLeft (MID_SPEED , 0);
      }
      else if (sharp_front_right > sharp_front) {
        motors.TurnRight (0 , MID_SPEED);
      } 
      else {
        motors.MoveForward (FULL_SPEED , FULL_SPEED);
      }
    } 
      
    else {
      SearchObject();
    }
  }
}
/* End of area cleaner section
--------------------------------------------------------------------------*/
/* Sumo section */

/* End of sumo section
--------------------------------------------------------------------------*/
/* Triggers section*/

void StartModalityTriggers() {
  // Calibration trigger
  if (current_screen == calibration){StartSprinterCalibration();}

  // Sumo trigger
  if (current_screen == modality && selected == sumo){/*StartSumoModality();*/}

  // Area cleaner trigger
  if (current_screen == flags && selected == areaCleaner){
    StartAreaCleanerSection();
  }

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

  // Begin display connection
  display.begin(SH1106_SWITCHCAPVCC, 0x3C);

  u8g2_for_adafruit_gfx.begin(display); // Begins u8g2 for gfx library

  pinMode(PIN_LED, OUTPUT);
  
  digitalWrite(PIN_LED, HIGH);

  pinMode(PIN_SELECT, INPUT_PULLUP);
  pinMode(PIN_DOWN, INPUT_PULLUP);

  // Displays team logo
  display.clearDisplay();
  display.drawBitmap( 0, 0, bitmap_alita, 128, 64, WHITE); // Prints teams logo
  display.display();

  delay(3000);
}

void loop() {
  //DisplayMenu();
  //StartModalityTriggers();
  StartAreaCleanerSection();
  //Telemetry();
  //SearchAllTheTrack();

  /*int datoBT;

  datoBT = SerialBT.read();


  SerialBT.print ("FULL = ");
  SerialBT.println (FULL_SPEED);
  SerialBT.print ("MID = ");
  SerialBT.println (MID_SPEED);
  SerialBT.print ("LOW = ");
  SerialBT.println (LOW_SPEED);
  SerialBT.print ("BARZOLA = ");
  SerialBT.println (BarzolaTurn);
  SerialBT.print ("SEARCH = ");
  SerialBT.println (SEARCH_OBJECT_TIME);

  if (datoBT == 'MM') {
    MID_SPEED = MID_SPEED + 10;
  } else if (datoBT == 'mM') {
    MID_SPEED = MID_SPEED - 10;
  }

  if (datoBT == 'MF') {
    FULL_SPEED = FULL_SPEED + 5;
  } else if (datoBT == 'mF') {
    FULL_SPEED = FULL_SPEED - 5;
  }

  if (datoBT == 'ML') {
    LOW_SPEED = LOW_SPEED + 5;
  } else if (datoBT == 'mL') {
    LOW_SPEED = LOW_SPEED - 5;
  }

  if (datoBT == 'MM') {
    TURN_SPEED = TURN_SPEED + 5;
  } else if (datoBT == 'mM') {
    TURN_SPEED = TURN_SPEED - 5;
  }

  if (datoBT == 'MB') {
    BarzolaTurn = BarzolaTurn + 100;
  } else if (datoBT == 'mB') {
    BarzolaTurn = BarzolaTurn - 100;
  }

  if (datoBT == 'MS') {
    SEARCH_OBJECT_TIME = SEARCH_OBJECT_TIME + 500;
  } else if (datoBT == 'mS') {
    SEARCH_OBJECT_TIME = SEARCH_OBJECT_TIME - 500;
  }*/
} 