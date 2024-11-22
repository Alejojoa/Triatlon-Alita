#include <Arduino.h>
#include "motor.h"

// Implementación de la clase Motor
Motor::Motor(int pin1, int pin2, int ch1, int ch2, int frequency, int resolution)
    : firstPin(pin1), secondPin(pin2), firstChannel(ch1), secondChannel(ch2), pwmFrequency(frequency), pwmResolution(resolution) {

    pinMode(firstPin, OUTPUT);
    pinMode(secondPin, OUTPUT);

    ledcSetup(firstChannel, pwmFrequency, pwmResolution);
    ledcSetup(secondChannel, pwmFrequency, pwmResolution);

    ledcAttachPin(firstPin, firstChannel);
    ledcAttachPin(secondPin, secondChannel);
}

void Motor::MoveForward(int speed) {
    ledcWrite(firstChannel, speed);
    ledcWrite(secondChannel, 0);
}

void Motor::MoveBackwards(int speed) {
    ledcWrite(firstChannel, 0);
    ledcWrite(secondChannel, speed);
}

void Motor::Brake() {
    ledcWrite(firstChannel, 255);
    ledcWrite(secondChannel, 255);
}

void Motor::StayStill() {
    ledcWrite(firstChannel, 0);
    ledcWrite(secondChannel, 0);
}

// Implementación de la clase MotorPair
MotorPair::MotorPair(int pinMr1, int pinMr2, int channelMr1, int channelMr2,
                     int pinMl1, int pinMl2, int channelMl1, int channelMl2, 
                     int frequency, int resolution)
    : motorRight(pinMr1, pinMr2, channelMr1, channelMr2, frequency, resolution),
      motorLeft(pinMl1, pinMl2, channelMl1, channelMl2, frequency, resolution) {
}

void MotorPair::MoveForward(int speedRight, int speedLeft) {
    motorRight.MoveForward(speedRight);
    motorLeft.MoveForward(speedLeft);
}

void MotorPair::MoveBackwards(int speedRight, int speedLeft) {
    motorRight.MoveBackwards(speedRight);
    motorLeft.MoveBackwards(speedLeft);
}

void MotorPair::TurnRight(int speedRight, int speedLeft) {
    motorRight.MoveBackwards(speedRight);
    motorLeft.MoveForward(speedLeft);
}

void MotorPair::TurnLeft(int speedRight, int speedLeft) {
    motorRight.MoveForward(speedRight);
    motorLeft.MoveBackwards(speedLeft);
}

void MotorPair::Brake() {
    motorRight.Brake();
    motorLeft.Brake();
}

void MotorPair::StayStill() {
    motorRight.StayStill();
    motorLeft.StayStill();
}