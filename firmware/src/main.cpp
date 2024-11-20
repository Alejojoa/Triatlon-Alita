#include <Adafruit_GFX.h>
#include <U8g2_for_Adafruit_GFX.h>
#include <CD74HC4067.h>
#include <Bluepad32.h>
#include <multiplexedQTR.h>
#include <bitmaps_triatlon.h>
#include <progress_bars.h>
#include <motor.h>

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

/* End of menu section
--------------------------------------------------------------------------*/
/* Sprinter section */
bool debug = true;

MotorPair motors(PIN_MR1, PIN_MR2, CHANNEL_MR1, CHANNEL_MR2, PIN_ML1, PIN_ML2,
                 CHANNEL_ML1, CHANNEL_ML2, PWM_FREQUENCY, PWM_RESOLUTION);

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

// PID const
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

void StartModalityTriggers()
{
  if (current_screen == flags && selected == calibration)
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

  // Begin display connection
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C))
  {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      ;
  }

  u8g2_for_adafruit_gfx.begin(display); // Begins u8g2 for gfx library

  BP32.setup(&onConnectedController, &onDisconnectedController);

  pinMode(PIN_SELECT, INPUT_PULLUP);
  pinMode(PIN_DOWN, INPUT_PULLUP);

  display.clearDisplay();
  display.drawBitmap(ALITA_BITMAP_MARGIN_X, ALITA_BITMAP_MARGIN_Y, bitmap_alita, ALITA_BITMAP_WIDTH, ALITA_BITMAP_HEIGHT, WHITE); // Prints teams logo
  display.display();

  delay(3000);
}

void loop()
{
  DisplayMenu();
  StartModalityTriggers();
}
