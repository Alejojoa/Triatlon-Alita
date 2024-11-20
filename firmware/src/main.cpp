#include <Adafruit_GFX.h>
#include <U8g2_for_Adafruit_GFX.h>
#include <CD74HC4067.h>
#include <motor.h>
#include <bitmaps_triatlon.h>
#include <progress_bars.h>
#include <multiplexedQTR.h>

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

CD74HC4067 my_mux(4, 25, 33, 32); // s0, s1, s2, s3

#define PIN_SIG 34

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

void StartModalityTriggers()
{
  if (current_screen == flags && selected == areaCleaner)
  {
    StartAreaCleanerSection();
  }
}

#define PIN_LED 23

void setup()
{
}

void loop()
{
  StartModalityTriggers();
}