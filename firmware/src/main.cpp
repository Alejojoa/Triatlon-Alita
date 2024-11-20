#include <Adafruit_GFX.h>
#include <U8g2_for_Adafruit_GFX.h>
#include <CD74HC4067.h>
#include <motor.h>
#include <bitmaps_triatlon.h>
#include <progress_bars.h>
#include <multiplexedQTR.h>

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

#define PIN_LED 23

void setup()
{
  SerialBT.begin("Alita");

  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, HIGH);
}

void loop()
{
  StartSprinterCalibration();
  StartSprinterModality();
}