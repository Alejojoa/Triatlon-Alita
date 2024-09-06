#include <arduino.h>
#include "locomotion.h"

Locomotion::Locomotion(int firstPinRight, int secondPinRight, int firstPinLeft, int secondPinLeft) 
    : motorRight(firstPinRight, secondPinRight), motorLeft(firstPinLeft, secondPinLeft) {}

void Locomotion::MoveForward(int speedRight, int speedLeft) {
    motorRight._MoveForward(speedRight);
    motorLeft._MoveForward(speedLeft);
}

void Locomotion::MoveBackwards(int speedRight, int speedLeft) {
    motorRight._MoveBackwards(speedRight);
    motorLeft._MoveBackwards(speedLeft);
}

void Locomotion::TurnRight(int speedRight, int speedLeft) {
    motorRight._MoveBackwards(speedRight);
    motorLeft._MoveForward(speedLeft);
}

void Locomotion::TurnLeft(int speedRight, int speedLeft) {
    motorRight._MoveForward(speedRight);
    motorLeft._MoveBackwards(speedLeft);
}

void Locomotion::StayStill() {
    motorRight._StayStill();
    motorLeft._StayStill();
}