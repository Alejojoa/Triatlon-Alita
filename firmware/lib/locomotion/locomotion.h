#ifndef LOCOMOTION_H
#define LOCOMOTION_H

#include "motor.h"

class Locomotion {
    private:
    Motor motorRight;
    Motor motorLeft;

    public:
    Locomotion(int firstPinRight, int secondPinRight, int firstPinLeft, int secondPinLeft);
    void MoveForward(int speedRight, int speedLeft);
    void MoveBackwards(int speedRight, int speedLeft);
    void TurnRight(int speedRight, int speedLeft);
    void TurnLeft(int speedRight, int speedLeft);
    void StayStill();

};

#endif