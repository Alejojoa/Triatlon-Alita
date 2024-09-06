#ifndef MOTOR_H
#define MOTOR_H

class Motor {
    private:
        int firstPin, secondPin;

    public:
        Motor(int firstPin, int secondPin);
        void _MoveForward(int speed);
        void _MoveBackwards(int speed);
        void _StayStill();

};

#endif