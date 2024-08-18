#include <arduino.h>
#include "locomotion.h"

Motor::Motor(int pin1, int pin2) {
    firstPin = pin1;
    secondPin = pin2;

    pinMode(firstPin, OUTPUT);
    pinMode(secondPin, OUTPUT);
}

void Motor::MoveForward(int speed) {
    analogWrite(firstPin, speed);
    analogWrite(secondPin, LOW);
}

void Motor::MoveBackwards(int speed) {
    analogWrite(firstPin, LOW);
    analogWrite(secondPin, speed);
}

void Motor::StayStill() {
    analogWrite(firstPin, LOW);
    analogWrite(secondPin, LOW);
}