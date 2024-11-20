#include <Bluepad32.h>

bool ctlConnected = false;

bool dataUpdated;

ControllerPtr myControllers[BP32_MAX_GAMEPADS];

Motor motorRight(26, 27, 0, 1, 1000, 8);
Motor motorLeft(16, 17, 2, 3, 1000, 8);

int axisXValue;
int throttleValue;
int brakeValue;

int throttlePWM;
int brakePWM;
int leftPWM;
int rightPWM;

// This callback gets called any time a new gamepad is connected.
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
  if (current_screen == modality && selected == sumo)
  {
    StartSumoModality();
  }
}

void setup()
{
  BP32.setup(&onConnectedController, &onDisconnectedController);
}

void loop()
{
  StartModalityTriggers();
}