#ifndef MOTOR_H
#define MOTOR_H

class Motor {
    private:
        int firstPin, secondPin;
        int firstChannel, secondChannel;
        int pwmFrequency;
        int pwmResolution;

    public:
        Motor(int firstPin, int secondPin, int firstChannel, int secondChannel, int frequency, int resolution);
        void MoveForward(int speed);
        void MoveBackwards(int speed);
        void Brake();
        void StayStill();
};

// Nueva clase MotorPair dentro del mismo archivo
class MotorPair {
    private:
        Motor motorRight, motorLeft;

    public:
        MotorPair(int pinMr1, int pinMr2, int channelMr1, int channelMr2,
                  int pinMl1, int pinMl2, int channelMl1, int channelMl2, 
                  int frequency, int resolution);

        void MoveForward(int speedRight, int speedLeft);
        void MoveBackwards(int speedRight, int speedLeft);
        void TurnRight(int speedRight, int speedLeft);
        void TurnLeft(int speedRight, int speedLeft);
        void Brake();
        void StayStill();
};

#endif