#include <Adafruit_GFX.h>
#include <U8g2_for_Adafruit_GFX.h>
#include <CD74HC4067.h>
#include <Bluepad32.h>
#include <multiplexedQTR.h>
#include <bitmaps_triatlon.h>
#include <progress_bars.h>
#include <motor.h>
#include <Bluepad32.h>

bool ctlConnected = false;

bool dataUpdated;

ControllerPtr myControllers[BP32_MAX_GAMEPADS];

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

Motor motorRight(26, 27, 0, 1, 1000, 8);
Motor motorLeft(16, 17, 2, 3, 1000, 8);

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1
#define SCREEN_ADDRESS 0x3C

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

#define SUMO_SCREEN_MARGIN_X 0
#define SUMO_SCREEN_MARGIN_Y 0
#define SUMO_SCREEN_WIDTH 128
#define SUMO_SCREEN_HEIGHT 64

#define FLAG_SCREEN_MARGIN_X 0
#define FLAG_SCREEN_MARGIN_Y 0
#define FLAG_SCREEN_WIDTH 128
#define FLAG_SCREEN_HEIGHT 64

#define SELECTED_BACKROUND_MARGIN_X 0
#define SELECTED_BACKROUND_MARGIN_Y 22
#define SELECTED_BACKROUND_WIDTH 128
#define SELECTED_BACKROUND_HEIGHT 20

#define ALITA_BITMAP_MARGIN_X 0
#define ALITA_BITMAP_MARGIN_Y 0
#define ALITA_BITMAP_WIDTH 128
#define ALITA_BITMAP_HEIGHT 64

#define ICON_SCREEN_MARGIN_X 4
#define ICON_SCREEN_WIDTH 16
#define ICON_SCREEN_HEIGHT 16

#define PREVIOUS_ICON_SCREEN_MARGIN_Y 2
#define SELECTED_ICON_SCREEN_MARGIN_Y 24
#define NEXT_ICON_SCREEN_MARGIN_Y 46

#define FIRST_SAFETY_TIMEOUT 3000
#define SECOND_SAFETY_TIMEOUT 2000

#define NUM_MODALITIES 3
#define MAX_ITEM_LENGTH 20

Adafruit_SH1106 display(OLED_RESET);
U8G2_FOR_ADAFRUIT_GFX u8g2_for_adafruit_gfx;

ProgressBar displayPB(display);

void DisplayMenu()
{
  if (current_screen == selection)
  {
    display.clearDisplay();

    u8g2_for_adafruit_gfx.setFontMode(1);
    u8g2_for_adafruit_gfx.setFontDirection(0);
    u8g2_for_adafruit_gfx.setForegroundColor(WHITE);
    u8g2_for_adafruit_gfx.setFont(u8g2_font_7x14_mf);
    u8g2_for_adafruit_gfx.setCursor(25, 15);
    u8g2_for_adafruit_gfx.print(F(menu_items[previous]));

    u8g2_for_adafruit_gfx.setFontMode(1);
    u8g2_for_adafruit_gfx.setFontDirection(0);
    u8g2_for_adafruit_gfx.setForegroundColor(WHITE);
    u8g2_for_adafruit_gfx.setFont(u8g2_font_7x14B_mf);
    u8g2_for_adafruit_gfx.setCursor(30, 37);
    u8g2_for_adafruit_gfx.print(F(menu_items[selected]));

    u8g2_for_adafruit_gfx.setFontMode(1);
    u8g2_for_adafruit_gfx.setFontDirection(0);
    u8g2_for_adafruit_gfx.setForegroundColor(WHITE);
    u8g2_for_adafruit_gfx.setFont(u8g2_font_7x14_mf);
    u8g2_for_adafruit_gfx.setCursor(25, 59);
    u8g2_for_adafruit_gfx.print(F(menu_items[next]));

    display.drawXBitmap(SELECTED_BACKROUND_MARGIN_X, SELECTED_BACKROUND_MARGIN_Y, epd_bitmap_selected_background, SELECTED_BACKROUND_WIDTH, SELECTED_BACKROUND_HEIGHT, WHITE);

    display.drawXBitmap(ICON_SCREEN_MARGIN_X, PREVIOUS_ICON_SCREEN_MARGIN_Y, bitmap_icons[previous], ICON_SCREEN_WIDTH, ICON_SCREEN_HEIGHT, WHITE);
    display.drawXBitmap(ICON_SCREEN_MARGIN_X, SELECTED_ICON_SCREEN_MARGIN_Y, bitmap_icons[selected], ICON_SCREEN_WIDTH, ICON_SCREEN_HEIGHT, WHITE);
    display.drawXBitmap(ICON_SCREEN_MARGIN_X, NEXT_ICON_SCREEN_MARGIN_Y, bitmap_icons[next], ICON_SCREEN_WIDTH, ICON_SCREEN_HEIGHT, WHITE);

    display.display();
  }
  else if (current_screen == modality && selected == sprinter)
  {
    displayPB.load(SPRINTER_SCREEN_MARGIN_X, SPRINTER_SCREEN_MARGIN_Y, SPRINTER_SCREEN_WIDTH, SPRINTER_SCREEN_HEIGHT, FIRST_SAFETY_TIMEOUT);
    displayPB.unload(SPRINTER_SCREEN_MARGIN_X, SPRINTER_SCREEN_MARGIN_Y, SPRINTER_SCREEN_WIDTH, SPRINTER_SCREEN_HEIGHT, SECOND_SAFETY_TIMEOUT);

    current_screen = flags;
  }
  else if (current_screen == modality && selected == areaCleaner)
  {
    displayPB.load(CLEANER_SCREEN_MARGIN_X, CLEANER_SCREEN_MARGIN_Y, CLEANER_SCREEN_WIDTH, CLEANER_SCREEN_HEIGHT, FIRST_SAFETY_TIMEOUT);
    displayPB.unload(CLEANER_SCREEN_MARGIN_X, CLEANER_SCREEN_MARGIN_Y, CLEANER_SCREEN_WIDTH, CLEANER_SCREEN_HEIGHT, SECOND_SAFETY_TIMEOUT);

    current_screen = flags;
  }
  else if (current_screen == flags)
  {
    display.clearDisplay();
    display.drawXBitmap(FLAG_SCREEN_MARGIN_X, FLAG_SCREEN_MARGIN_Y, epd_bitmap_flag, FLAG_SCREEN_WIDTH, FLAG_SCREEN_HEIGHT, WHITE);
    display.display();
  }
  else if (current_screen == modality && selected == sumo)
  {
    if (!ctlConnected)
    {
      display.clearDisplay();
      display.drawXBitmap(SUMO_SCREEN_MARGIN_X, SUMO_SCREEN_MARGIN_Y, bitmap_screens[selected], SUMO_SCREEN_WIDTH, SUMO_SCREEN_HEIGHT, WHITE);
      display.display();

      delay(250);

      display.clearDisplay();
      display.display();

      delay(250);
    }
    else
    {
      display.clearDisplay();
      display.drawXBitmap(SUMO_SCREEN_MARGIN_X, SUMO_SCREEN_MARGIN_Y, bitmap_screens[selected], SUMO_SCREEN_WIDTH, SUMO_SCREEN_HEIGHT, WHITE);
      display.display();
    }
  }
  else if (current_screen == modality && selected == sprinter)
  {
    display.clearDisplay();
    display.drawXBitmap(0, 0, bitmap_screens[selected], 128, 64, WHITE);
    display.display();
  }

  UpdateScreenStatus();

  if (current_screen == selection)
  {
    motors.StayStill();

    motorRight.StayStill();
    motorLeft.StayStill();

    BP32.enableNewBluetoothConnections(false);
  }
}

multiplexedQTR qtr;

const uint8_t SensorCount = 8;
uint16_t sensorValues[SensorCount];

int position;

int getPosition()
{
  position = qtr.readLineWhite(sensorValues);
  return position;
}

void StartSprinterCalibration()
{
  qtr.setTypeAnalog();
  qtr.setSensorPins((const uint8_t[]){0, 1, 2, 3, 4, 5, 6, 7}, SensorCount);

  delay(500);

  display.clearDisplay();
  display.drawXBitmap(0, 0, bitmap_calibration, 124, 64, WHITE);
  display.display();

  for (uint16_t i = 0; i < 350; i++)
  {
    qtr.calibrate();
  }

  current_screen = modality;
}

int setPoint = 1750;

int proportional;
int derivative;
int integral;
int lastError;

int maxSpeed = 180;
int minSpeed = 140;
int speed = 160;

float kp = 0.013;
float ki = 0;
float kd = 0;
float pid;
float pidRight;
float pidLeft;

void StartSprinterModality()
{
  position = getPosition();

  proportional = position - setPoint;
  derivative = proportional - lastError;

  pid = (proportional * kp) + (derivative * kd);

  lastError = proportional;

  pidRight = speed + pid;
  pidLeft = speed - pid;

  if (pidRight > maxSpeed)
  {
    pidRight = maxSpeed;
  }
  if (pidLeft > maxSpeed)
  {
    pidLeft = maxSpeed;
  }

  if (pidRight <= minSpeed && pidLeft > minSpeed)
  {
    motors.TurnRight(minSpeed + (minSpeed - pidRight), pidLeft);
  }
  else if (pidLeft <= minSpeed && pidRight > minSpeed)
  {
    motors.TurnLeft(pidRight, minSpeed + (minSpeed - pidLeft));
  }
  else
  {
    motors.MoveForward(pidRight, pidLeft);
  }
}

#define QRE_BLACK 3900
#define SHARP_ATTACK 1000
#define SET_OUT_BACKWARDS_TIME 250

int LOW_SPEED = 85;
int MID_SPEED = 95;
int FULL_SPEED = 110;

bool offRoad;

int sharp_left;
int sharp_right;
int sharp_front_right;
int sharp_front;
int sharp_front_left;
int qre_back;
int qre_left;
int qre_right;

int signal_input;

#define PIN_S0 4
#define PIN_S1 25
#define PIN_S2 33
#define PIN_S3 32
#define PIN_SIG 34

CD74HC4067 my_mux(PIN_S0, PIN_S1, PIN_S2, PIN_S3);

void ReadCleanerSensors()
{

  for (int x = 8; x < 15; x++)
  {

    my_mux.channel(x);

    signal_input = analogRead(PIN_SIG);

    switch (x)
    {

    case 8:
    {
      qre_left = signal_input;
      break;
    }
    case 9:
    {
      qre_back = signal_input;
      break;
    }
    case 10:
    {
      qre_right = signal_input;
      break;
    }
    case 11:
    {
      sharp_right = signal_input;
      break;
    }
    case 12:
    {
      sharp_front_right = signal_input;
      break;
    }
    case 13:
    {
      sharp_front = signal_input;
      break;
    }
    case 14:
    {
      sharp_front_left = signal_input;
      break;
    }
    case 15:
    {
      sharp_left = signal_input;
      break;
    }
    }
  }
}

void CheckOffRoad()
{
  ReadCleanerSensors();

  if (qre_left > QRE_BLACK || qre_right > QRE_BLACK || qre_back > QRE_BLACK)
  {
    offRoad = true;
  }
  else
  {
    offRoad = false;
  }
}

bool out_lastMovment;
#define BarzolaTurn 1000
#define SafetyStop 700

void Out()
{
  ReadCleanerSensors();

  unsigned long CurrentTime_OUT;

  if (qre_left > QRE_BLACK || qre_right > QRE_BLACK)
  {
    CurrentTime_OUT = millis();

    while (millis() < CurrentTime_OUT + SET_OUT_BACKWARDS_TIME)
    {
      ReadCleanerSensors();
      CheckOffRoad();
      motors.MoveBackwards(FULL_SPEED, FULL_SPEED);
      if (qre_back > QRE_BLACK)
      {
        break;
      }
      delay(10);
    }

    while (millis() < CurrentTime_OUT + BarzolaTurn)
    {
      ReadCleanerSensors();
      CheckOffRoad();
      motors.TurnRight(LOW_SPEED, LOW_SPEED);
      if (sharp_front_left > SHARP_ATTACK || sharp_front > SHARP_ATTACK || sharp_front_right > SHARP_ATTACK)
      {
        break;
      }
      if (offRoad)
      {
        break;
      }
      delay(10);
    }

    out_lastMovment = true;
    CheckOffRoad();
  }

  if (qre_back > QRE_BLACK)
  {
    CurrentTime_OUT = millis();

    if (out_lastMovment)
    {
      while (millis() < CurrentTime_OUT + 200)
      {
        ReadCleanerSensors();
        CheckOffRoad();
        motors.MoveForward(MID_SPEED, MID_SPEED);
        delay(10);
      }
    }
    else if (!out_lastMovment)
    {
      while (millis() < CurrentTime_OUT + SET_OUT_BACKWARDS_TIME)
      {
        ReadCleanerSensors();
        CheckOffRoad();
        motors.MoveBackwards(FULL_SPEED, FULL_SPEED);
        delay(10);
      }

      while (millis() < CurrentTime_OUT + SafetyStop)
      {
        ReadCleanerSensors();
        CheckOffRoad();
        motors.Brake();
        delay(10);
      }
    }
  }
}

bool state_found_left;
bool state_found_right;
bool object_back;
long time_found_left;
long time_found_right;

void OutObjectBack()
{
  ReadCleanerSensors();
  CheckOffRoad();
  long CurrentTime_ObjectBack;
  int total_found_left;
  int total_found_right;

  if (state_found_left)
  {
    total_found_left = millis() - (time_found_left + SET_OUT_BACKWARDS_TIME);
  }
  if (state_found_right)
  {
    total_found_right = millis() - (time_found_right + SET_OUT_BACKWARDS_TIME);
  }

  if (state_found_left && !state_found_right)
  {
    while (millis() < CurrentTime_ObjectBack + total_found_left)
    {
      motors.MoveBackwards(MID_SPEED, MID_SPEED);
      delay(10);
    }
  }
  else if (!state_found_left && state_found_right)
  {
    while (millis() < CurrentTime_ObjectBack + total_found_right)
    {
      motors.MoveBackwards(MID_SPEED, MID_SPEED);
      delay(10);
    }
  }
  else if (state_found_left && state_found_right)
  {
    if (total_found_left < total_found_right)
    {
      while (millis() < CurrentTime_ObjectBack + total_found_left)
      {
        motors.MoveBackwards(MID_SPEED, MID_SPEED);
        delay(10);
      }
    }
    else if (total_found_left > total_found_right)
    {
      while (millis() < CurrentTime_ObjectBack + total_found_right)
      {
        motors.MoveBackwards(MID_SPEED, MID_SPEED);
        delay(10);
      }
    }
  }

  object_back = false;
  state_found_left = false;
  state_found_right = false;
  total_found_left = 0;
  total_found_right = 0;
  time_found_left = 0;
  time_found_right = 0;
}

void SearchAllTheTrack()
{
  ReadCleanerSensors();
  CheckOffRoad();
  long CurrentTime_SearchAllTheTrack;

  CurrentTime_SearchAllTheTrack = millis();

  if (!offRoad)
  {
    motors.MoveForward(MID_SPEED, MID_SPEED);
  }
  else
  {
    motors.TurnRight(LOW_SPEED, MID_SPEED);
  }
}

void SearchObject()
{

  ReadCleanerSensors();
  CheckOffRoad();

  long CurrentTime_SearchObject = millis();

  for (int i = 0; i < 18; i++)
  {
    long CurrentTime_Brake = millis() + 200;
    long CurrentTime_TurnRight = millis() + 100;

    while (millis() < CurrentTime_TurnRight)
    {
      motors.TurnRight(MID_SPEED, MID_SPEED);
      ReadCleanerSensors();
      if (sharp_front_left > SHARP_ATTACK || sharp_front > SHARP_ATTACK || sharp_front_right > SHARP_ATTACK)
      {
        break;
      }
      if (offRoad)
      {
        break;
      }
    }
    while (millis() < CurrentTime_Brake)
    {
      motors.Brake();
      ReadCleanerSensors();
      if (sharp_front_left > SHARP_ATTACK || sharp_front > SHARP_ATTACK || sharp_front_right > SHARP_ATTACK)
      {
        break;
      }
      if (sharp_right > SHARP_ATTACK)
      {
        break;
      }
      if (offRoad)
      {
        break;
      }
    }

    if (sharp_front_left > SHARP_ATTACK || sharp_front > SHARP_ATTACK || sharp_front_right > SHARP_ATTACK)
    {
      break;
    }
    if (offRoad)
    {
      break;
    }

    i = i + 1;
  }

  while (millis() < CurrentTime_SearchObject + 2000)
  {
    ReadCleanerSensors();
    CheckOffRoad();
    motors.MoveForward(MID_SPEED, MID_SPEED);

    if (sharp_front_left > SHARP_ATTACK || sharp_front > SHARP_ATTACK || sharp_front_right > SHARP_ATTACK)
    {
      break;
    }

    if (offRoad)
    {
      break;
    }
    delay(10);
  }
}

void StartAreaCleanerSection()
{
  ReadCleanerSensors();
  CheckOffRoad();

  if (offRoad)
  {
    Out();
  }

  else
  {
    if (sharp_front_left > SHARP_ATTACK || sharp_front > SHARP_ATTACK || sharp_front_right > SHARP_ATTACK)
    {

      out_lastMovment = false;

      if (sharp_front_left > sharp_front)
      {
        motors.TurnLeft(MID_SPEED, 0);
      }
      else if (sharp_front_right > sharp_front)
      {
        motors.TurnRight(0, MID_SPEED);
      }
      else
      {
        ReadCleanerSensors();
        CheckOffRoad();

        motors.MoveForward(FULL_SPEED, FULL_SPEED);
      }
    }

    else
    {
      SearchObject();
    }
  }
}

int axisXValue;
int throttleValue;
int brakeValue;

int throttlePWM;
int brakePWM;
int leftPWM;
int rightPWM;

void onConnectedController(ControllerPtr ctl)
{

  bool foundEmptySlot = false;

  ctlConnected = true;

  if (myControllers[0] == nullptr)
  {
    Serial.printf("CALLBACK: Controller is connected, index=%d\n");
    ControllerProperties properties = ctl->getProperties();
    Serial.printf("Controller model: %s, VID=0x%04x, PID=0x%04x\n", ctl->getModelName().c_str(), properties.vendor_id, properties.product_id);
    myControllers[0] = ctl;
    foundEmptySlot = true;
  }

  if (!foundEmptySlot)
  {
    Serial.println("CALLBACK: Controller connected, but could not found empty slot");
  }
}

void onDisconnectedController(ControllerPtr ctl)
{
  bool foundController = false;

  ctlConnected = false;

  if (myControllers[0] == ctl)
  {
    Serial.printf("CALLBACK: Controller disconnected from index=%d\n");
    myControllers[0] = nullptr;
    foundController = true;
  }

  if (!foundController)
  {
    Serial.println("CALLBACK: Controller disconnected, but not found in myControllers");
  }
}

void processGamepad(ControllerPtr ctl)
{
  bool brakeRight;
  bool brakeLeft;

  int yAxisValueR = ctl->axisRY();
  int yAxisValueL = ctl->axisY();

  int RBValue = ctl->throttle();
  int LBValue = ctl->brake();

  int rightWheelSpeedF = map(yAxisValueR, 0, -511, 0, 255);
  int rightWheelSpeedB = map(yAxisValueR, 0, 512, 0, 255);

  int leftWheelSpeedF = map(yAxisValueL, 0, -511, 0, 255);
  int leftWheelSpeedB = map(yAxisValueL, 0, 512, 0, 255);

  int turnRightSpeed = map(RBValue, 0, 1023, 0, 255);
  int turnLeftSpeed = map(LBValue, 0, 1023, 0, 255);

  if (ctl->r1())
  {
    brakeRight = true;
  }
  else
  {
    brakeRight = false;
  }
  if (ctl->l1())
  {
    brakeLeft = true;
  }
  else
  {
    brakeLeft = false;
  }

  if (!brakeRight)
  {
    if (yAxisValueR < -70)
    {
      motorRight.MoveForward(rightWheelSpeedF);
    }
    else if (yAxisValueR > 70)
    {
      motorRight.MoveBackwards(rightWheelSpeedB);
    }
    else
    {
      motorRight.StayStill();
    }
  }
  else
  {
    motorRight.StayStill();
  }

  if (!brakeLeft)
  {
    if (yAxisValueL < -70)
    {
      motorLeft.MoveForward(leftWheelSpeedF);
    }
    else if (yAxisValueL > 70)
    {
      motorLeft.MoveBackwards(leftWheelSpeedB);
    }
    else
    {
      motorLeft.StayStill();
    }
  }
  else
  {
    motorLeft.StayStill();
  }

  if (RBValue > 150)
  {
    motorRight.MoveBackwards(turnRightSpeed);
    motorLeft.MoveForward(turnRightSpeed);
  }

  if (LBValue > 150)
  {
    motorRight.MoveForward(turnLeftSpeed);
    motorLeft.MoveBackwards(turnLeftSpeed);
  }
}

void processControllers()
{
  for (auto myController : myControllers)
  {
    if (myController && myController->isConnected() && myController->hasData())
    {
      processGamepad(myController);
    }
  }
}

void StartSumoModality()
{
  BP32.enableNewBluetoothConnections(true);

  dataUpdated = BP32.update();

  if (dataUpdated)
  {
    processControllers();
  }
}

void StartModalityTriggers()
{
  if (current_screen == flags && selected == areaCleaner)
  {
    StartAreaCleanerSection();
  }
  else if (current_screen == modality && selected == sumo)
  {
    StartSumoModality();
  }
  else if (current_screen == calibration)
  {
    StartSprinterCalibration();
  }
  else if (current_screen == flags && selected == sprinter)
  {
    StartSprinterModality();
  }
}

#define PIN_LED 23

void setup()
{
  display.begin(SH1106_SWITCHCAPVCC, SCREEN_ADDRESS);

  u8g2_for_adafruit_gfx.begin(display);

  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, HIGH);

  BP32.setup(&onConnectedController, &onDisconnectedController);

  pinMode(PIN_SELECT, INPUT_PULLUP);
  pinMode(PIN_DOWN, INPUT_PULLUP);

  display.clearDisplay();
  display.drawBitmap(ALITA_BITMAP_MARGIN_X, ALITA_BITMAP_MARGIN_Y, bitmap_alita, ALITA_BITMAP_WIDTH, ALITA_BITMAP_HEIGHT, WHITE);
  display.display();

  delay(3000);
}

void loop()
{
  DisplayMenu();
  StartModalityTriggers();
}