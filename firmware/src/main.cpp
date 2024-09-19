#include <Arduino.h>
#include <CD74HC4067.h>
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <BluetoothSerial.h>

#define PIN_LED 23

// Definicion de motores 
#define PIN_MA1 26
#define PIN_MA2  27
#define PIN_MB1  17
#define PIN_MB2  16

#define PIN_SIG 34

// define display size in pixels
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

#define PIN_SELECT 18

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Sharp
int sharp_left;
int sharp_right;
int sharp_front_right;
int sharp_front;
int sharp_front_left;

int low_speed = 100;
int mid_speed = 150;
int full_speed = 200;

int signal_input;

int mux_ch;

BluetoothSerial SerialBT;

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth in not enabled! Plese run 'make menuconfig' to and enable it
#endif

CD74HC4067 my_mux(4, 25, 33, 32); // s0, s1, s2, s3

void setup() {

  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);
  
  }

  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, HIGH);

  pinMode(PIN_MA1, OUTPUT);
  pinMode(PIN_MA2, OUTPUT);
  pinMode(PIN_MB1, OUTPUT);
  pinMode(PIN_MB2, OUTPUT);

  pinMode(PIN_SIG, INPUT);

  pinMode(PIN_SELECT, INPUT_PULLUP);

  mux_ch = 8;

}

void loop() {

  //15, 14, 13 = QRELeft, back, right
  //12, 11, 10, 9, 8 = SHARPLeft, frontLeft, front, frontRight, right

  my_mux.channel(15);
  signal_input = analogRead(PIN_SIG);

  display.clearDisplay();
  display.setCursor(0, 40);
  display.setTextColor(WHITE);
  display.setTextSize(3);
  display.print(signal_input);
  display.display();

}