#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>
#include <QTRSensors.h>
#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is no enabled! Please run `make menuconfig` to and enable it
#endif

bool run = false;

#define MA1 27
#define MA2 26
#define MB1 18
#define MB2 17

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)
#define SCREEN_ADDRESS 0x3C
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

const int S0 = 14;
const int S1 = 13;
const int S2 = 12;
const int S3 = 11;

int setPoint = 3500;

int proportional = 0;
int derivative = 0;
int integral = 0;
int lastErr = 0;

int maxSpeed = 255;
int minSpeed = 130;

int velocity = 255;
int turnVelocity = 130;

float kp = 0;
float ki = 0;
float kd = 0;

float speed = 0;

float pidRight = 0;
float pidLeft = 0;

BluetoothSerial SerialBT;

QTRSensors qtr;

const uint8_t SensorCount = 8;
uint16_t sensorValues[SensorCount];

int getPosition (){
    int position = qtr.readLineWhite(sensorValues);
    return position;
}

void multiplexer(){
    pinMode (S0, OUTPUT);
    pinMode (S1, OUTPUT);
    pinMode (S2, OUTPUT);
    pinMode (S3, OUTPUT);
}


void calibration(){
    qtr.setTypeAnalog();
    //qtr.setSensorPins((const uint8_t[]){pin1, pin2, pin3...}, SensorCount);
    delay(500);

    for (uint16_t i = 0; i < 300; i++){
        qtr.calibrate();
    }
}

void motors(){
    pinMode(MA1, OUTPUT);
    pinMode(MA2, OUTPUT);
    pinMode(MB1, OUTPUT);
    pinMode(MB2, OUTPUT);
}

void forw(){
    analogWrite(MA1, pidRight);
    analogWrite(MA2, 0);
    analogWrite(MB1, pidLeft);
    analogWrite(MB2, 0);
}

void still(){
    analogWrite(MA1, 0);
    analogWrite(MA2, 0);
    analogWrite(MB1, 0);
    analogWrite(MB2, 0);
}

void PID (){
    int position = getPosition();

    proportional = position - setPoint;
    integral = integral + proportional;
    derivative = proportional - lastErr;
    
    speed = (proportional * kp) + (integral * ki) + (derivative * kd);
    
    lastErr = proportional;

    pidRight = velocity - speed;
    pidLeft = velocity + speed;

    if (pidRight > maxSpeed){
        pidRight = maxSpeed;
    } else if (pidRight < minSpeed){
        pidRight = minSpeed;
    }

    if (pidLeft > maxSpeed){
        pidLeft = maxSpeed;
    } else if (pidLeft < minSpeed){
        pidLeft = minSpeed;
    }

    forw();

}

void setup(){
    multiplexer();
    
    display.begin(SSD1306_SWITCHCAPVCC, 0x3C);
    display.clearDisplay();
    display.setTextColor(WHITE);
    display.setTextSize(2);
    display.setCursor(60, 10);
    SerialBT.begin("Alita");
    display.println("Waiting for Bluetooth...");

    if (!(CONFIG_BT_ENABLED) || !(CONFIG_BLUEDROID_ENABLED))
        display.clearDisplay();
        display.setTextColor(WHITE);
        display.setTextSize(1);
        display.setCursor(0, 0);
        display.println("Unable to connect. Please run `make menuconfig` to and enable it");
    
    delay(500);

    calibration();

    motors();

}

void loop() {

PID();

}

// 89 porque la velocidad pid es la velocidad maxima menos la formula pid en vez de simplemente la formula?
// chanel de cada motor
// bool 