#include <Adafruit_SSD1306.h> 
#include <Adafruit_GFX.h>
#include <U8g2_for_Adafruit_GFX.h>
#include <CD74HC4067.h>
#include <Bluepad32.h>
#include <multiplexedQTR.h>
#include <locomotion.h>
#include <bitmaps_triatlon.h>
#include <progress_bars.h>

/* Global section
--------------------------------------------------------------------------*/

bool ctlConnected = false;

bool dataUpdated;

ControllerPtr myControllers[BP32_MAX_GAMEPADS];

Motor motorRight(26, 27);
Motor motorLeft(16, 17);

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
    if (!ctlConnected) {
      display.clearDisplay();
      display.drawXBitmap( 0, 0, bitmap_screens[selected], 128, 64, WHITE);
      display.display();

      delay(250);

      display.clearDisplay();
      display.display();

      delay(250);
    } else {
      display.clearDisplay();
      display.drawXBitmap( 0, 0, bitmap_screens[selected], 128, 64, WHITE);
      display.display();
    }
  }
  else if (current_screen == modality && selected == sprinter){
    display.clearDisplay();
    display.drawXBitmap( 0, 0, bitmap_screens[selected], 128, 64, WHITE);
    display.display();
  }

  UpdateScreenStatus();

  if (current_screen == selection){  
    motorRight.StayStill();
    motorLeft.StayStill();

    BP32.enableNewBluetoothConnections(false);
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

int setPoint = 5200; // Sets line position

int proportional;
int derivative;
int integral;
int lastError;

int maxSpeed = 180;
int minSpeed = 140;
int speed = 160;

// PID const
float kp = 0.013;
float ki = 0;
float kd = 0;
float pid;
float pidRight;
float pidLeft;

//PID control system code
void StartSprinterModality(){
  position = getPosition();

  proportional = position - setPoint; // Newest error
  integral += proportional; // Integral of the error
  derivative = proportional - lastError; // Derivative of the error

  // PID aftermath
  pid = (proportional * kp) + (integral * ki) + (derivative * kd);
    
  lastError = proportional; // Saves last error

  pidRight = speed + pid;
  pidLeft = speed - pid;

  // Defines speed limits for right motor
  if (pidRight > maxSpeed){pidRight = maxSpeed;} 
  else if (pidRight < minSpeed){pidRight = minSpeed;}

  // Defines speed limits for left motor
  if (pidLeft > maxSpeed){pidLeft = maxSpeed;} 
  else if (pidLeft < minSpeed){pidLeft = minSpeed;}

  // Defines turning speed
  if (pidRight <= minSpeed&& pidLeft > minSpeed){ // Turns right 
    motorRight.MoveBackwards(pidRight);
    motorLeft.MoveForward(pidLeft);
  } else if (pidLeft <= minSpeed && pidRight > minSpeed){ // Turns left
    motorRight.MoveForward(pidRight);
    motorLeft.MoveBackwards(pidLeft);
  } else { // Goes stright
    motorRight.MoveForward(pidRight);
    motorLeft.MoveForward(pidLeft);
  }
}

/* End of sprinter section
--------------------------------------------------------------------------*/
/* Area cleaner section */

// Cuando Iván lo considere listo lo pone

/* End of area cleaner section
--------------------------------------------------------------------------*/
/* Sumo section */

int axisXValue;
int throttleValue ;
int brakeValue;

int throttlePWM;
int brakePWM;
int leftPWM;
int rightPWM;

// This callback gets called any time a new gamepad is connected.
void onConnectedController(ControllerPtr ctl) {
  
  bool foundEmptySlot = false;

  ctlConnected = true;
  
  if (myControllers[0] == nullptr) {
    Serial.printf("CALLBACK: Controller is connected, index=%d\n");
    ControllerProperties properties = ctl->getProperties();
    Serial.printf("Controller model: %s, VID=0x%04x, PID=0x%04x\n", ctl->getModelName().c_str(), properties.vendor_id, properties.product_id);
    myControllers[0] = ctl;
    foundEmptySlot = true;
  }

  if (!foundEmptySlot) {
    Serial.println("CALLBACK: Controller connected, but could not found empty slot");
  }
}

void onDisconnectedController(ControllerPtr ctl) {
  bool foundController = false;

  ctlConnected = false;
    
  if (myControllers[0] == ctl) {
    Serial.printf("CALLBACK: Controller disconnected from index=%d\n");
    myControllers[0] = nullptr;
    foundController = true;
  } 

  if (!foundController) {
    Serial.println("CALLBACK: Controller disconnected, but not found in myControllers");
  }
}

void processGamepad(ControllerPtr ctl) {
  bool brakeRight;
  bool brakeLeft;

  // Variables to store the input of each stick
  int yAxisValueR = ctl->axisRY();
  int yAxisValueL = ctl->axisY();

  int RBValue = ctl->throttle();
  int LBValue = ctl->brake();

  // Mapping of each stick input to 8bit 
  int rightWheelSpeedF = map(yAxisValueR, 0, -511, 0, 255);
  int rightWheelSpeedB = map(yAxisValueR, 0, 512, 0, 255);
  
  int leftWheelSpeedF = map(yAxisValueL, 0, -511, 0, 255);
  int leftWheelSpeedB = map(yAxisValueL, 0, 512, 0, 255);

  int turnRightSpeed = map(RBValue, 0, 1023, 0, 255);
  int turnLeftSpeed = map(LBValue, 0, 1023, 0, 255);

  if (ctl->r1()) {brakeRight = true;}
  else {brakeRight = false;}
  if (ctl->l1()) {brakeLeft = true;}
  else {brakeLeft = false;}

  if (!brakeRight) {
    if (yAxisValueR < -70) {motorRight.MoveForward(rightWheelSpeedF);}
    else if (yAxisValueR > 70) {motorRight.MoveBackwards(rightWheelSpeedB);}
    else {motorRight.StayStill();}
  }
  else {motorRight.StayStill();}

  if (!brakeLeft) {
    if (yAxisValueL < -70) {motorLeft.MoveForward(leftWheelSpeedF);}
    else if (yAxisValueL > 70) {motorLeft.MoveBackwards(leftWheelSpeedB);}
    else {motorLeft.StayStill();}
  } 
  else {motorLeft.StayStill();}

  if (RBValue > 150) {
    motorRight.MoveBackwards(turnRightSpeed);
    motorLeft.MoveForward(turnRightSpeed);
  }

  if (LBValue > 150) {
    motorRight.MoveForward(turnLeftSpeed);
    motorLeft.MoveBackwards(turnLeftSpeed);
  }
}

void processControllers() {
  for (auto myController : myControllers) {
    if (myController && myController->isConnected() && myController->hasData()) {
      processGamepad(myController);
    }
  }
}

void StartSumoModality(){
  BP32.enableNewBluetoothConnections(true);

  dataUpdated = BP32.update();
  
  if (dataUpdated) {
    processControllers();
  }
}

/* End of sumo section
--------------------------------------------------------------------------*/
/* Triggers section*/

void StartModalityTriggers() {
  // Calibration trigger
  if (current_screen == calibration){StartSprinterCalibration();}

  // Sumo trigger
  if (current_screen == modality && selected == sumo){StartSumoModality();}

  // Area cleaner trigger
  if (current_screen == flags && selected == areaCleaner){/*StartAreaCleanerModality();*/}

  // Sprinter trigger
  else if (current_screen == flags && selected == sprinter){
    StartSprinterModality();
  }
}

/* End of triggers section
--------------------------------------------------------------------------*/
/* Setup and loop section */

#define PIN_LED 23

void setup(){    
  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, HIGH);

  // Begin display connection
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {    
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);  
  }

  u8g2_for_adafruit_gfx.begin(display); // Begins u8g2 for gfx library

  BP32.setup(&onConnectedController, &onDisconnectedController);

  pinMode(PIN_SELECT, INPUT_PULLUP);
  pinMode(PIN_DOWN, INPUT_PULLUP);

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