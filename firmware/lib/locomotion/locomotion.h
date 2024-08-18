#ifndef LOCOMOTION_H
#define LOCOMOTION_H

class Motor {
    private:
        int firstPin, secondPin;

    public:
        Motor(int firstPin, int secondPin);
        void MoveForward(int speed);
        void MoveBackwards(int speed);
        void StayStill();

};

#endif