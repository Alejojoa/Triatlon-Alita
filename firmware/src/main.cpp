#include <Adafruit_SSD1306.h> 
#include <Adafruit_GFX.h>
#include <U8g2_for_Adafruit_GFX.h>
#include <CD74HC4067.h>
#include <motor.h>
#include <bitmaps_triatlon.h>
#include <progress_bars.h>
#include <multiplexedQTR.h>
#include "BluetoothSerial.h"

#define PIN_MR1 26 //primer pin motor derecho
#define PIN_MR2 27 //segundo pin motor derecho
#define PIN_ML1 16 //primer pin motor izquierdo
#define PIN_ML2 17 //segundo pin motor izquierdo

#define CHANNEL_MR1 0 //primer canal motor derecho
#define CHANNEL_MR2 1 //segundo canal motor derecho
#define CHANNEL_ML1 2 //primer canal motor izquierdo
#define CHANNEL_ML2 3 //segundo canal motor izquierdo

#define PWM_FREQUENCY 1000 //frecuencia de pulso de pwm
#define PWM_RESOLUTION 8 //resolución en bits del pwm

bool debug = true;

//Creamos el objeto motors con el metodo MotorPair de la clase Motors con sus pines, canales, frecuencia y resolución
MotorPair motors(PIN_MR1, PIN_MR2, CHANNEL_MR1, CHANNEL_MR2, PIN_ML1, PIN_ML2, 
                 CHANNEL_ML1, CHANNEL_ML2, PWM_FREQUENCY, PWM_RESOLUTION); 

BluetoothSerial SerialBT;

// Define display size in pixels
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

// Define display buttons 
#define PIN_SELECT 18
#define PIN_DOWN 5

#define SPRINTER_SCREEN_MARGIN_X 35
#define SPRINTER_SCREEN_MARGIN_Y 0
#define SPRINTER_SCREEN_WIDTH 60
#define SPRINTER_SCREEN_HEIGHT 64

#define CLEANER_SCREEN_MARGIN_X 0
#define CLEANER_SCREEN_MARGIN_Y 0
#define CLEANER_SCREEN_WIDTH 128
#define CLEANER_SCREEN_HEIGHT 64

#define FIRST_SAFETY_TIMEOUT 3000
#define SECOND_SAFETY_TIMEOUT 2000

#define NUM_MODALITIES 3
#define MAX_ITEM_LENGTH 20

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
U8G2_FOR_ADAFRUIT_GFX u8g2_for_adafruit_gfx;

ProgressBar displayPB(display);

// Prints a menu
void DisplayMenu(){
  if (current_screen == selection) {
    display.clearDisplay();

    // Prints previous item name
    u8g2_for_adafruit_gfx.setFontMode(1);
    u8g2_for_adafruit_gfx.setFontDirection(0);
    u8g2_for_adafruit_gfx.setForegroundColor(WHITE);
    u8g2_for_adafruit_gfx.setFont(u8g2_font_7x14_mf);
    u8g2_for_adafruit_gfx.setCursor(25, 15);
    u8g2_for_adafruit_gfx.print(F(menu_items [previous]));

    // Prints selected item name
    u8g2_for_adafruit_gfx.setFontMode(1);
    u8g2_for_adafruit_gfx.setFontDirection(0);
    u8g2_for_adafruit_gfx.setForegroundColor(WHITE);
    u8g2_for_adafruit_gfx.setFont(u8g2_font_7x14B_mf);
    u8g2_for_adafruit_gfx.setCursor(30, 37);
    u8g2_for_adafruit_gfx.print(F(menu_items [selected]));
  
    // Prints next item name
    u8g2_for_adafruit_gfx.setFontMode(1);
    u8g2_for_adafruit_gfx.setFontDirection(0);
    u8g2_for_adafruit_gfx.setForegroundColor(WHITE);
    u8g2_for_adafruit_gfx.setFont(u8g2_font_7x14_mf);
    u8g2_for_adafruit_gfx.setCursor(25, 59);
    u8g2_for_adafruit_gfx.print(F(menu_items [next]));
    
    // Prints selection frame
    display.drawXBitmap ( 0, 22, epd_bitmap_selected_background, 128, 20, WHITE);
    
    // Prints previous item icon
    display.drawXBitmap ( 4, 2, bitmap_icons[previous], 16, 16, WHITE);

    // Prints selected item icon
    display.drawXBitmap ( 4, 24, bitmap_icons[selected], 16, 16, WHITE);

    // Prints next item icon
    display.drawXBitmap ( 4, 46, bitmap_icons[next], 16, 16, WHITE);

    display.display();
  } 
  else if (current_screen == modality && selected == sprinter) {
    displayPB.load(SPRINTER_SCREEN_MARGIN_X, SPRINTER_SCREEN_MARGIN_Y, SPRINTER_SCREEN_WIDTH, SPRINTER_SCREEN_HEIGHT, FIRST_SAFETY_TIMEOUT);
    displayPB.unload(SPRINTER_SCREEN_MARGIN_X, SPRINTER_SCREEN_MARGIN_Y, SPRINTER_SCREEN_WIDTH, SPRINTER_SCREEN_HEIGHT, SECOND_SAFETY_TIMEOUT);

    current_screen = flags;
  }
  else if (current_screen == modality && selected == areaCleaner) {
    displayPB.load(CLEANER_SCREEN_MARGIN_X, CLEANER_SCREEN_MARGIN_Y, CLEANER_SCREEN_WIDTH, CLEANER_SCREEN_HEIGHT, FIRST_SAFETY_TIMEOUT);   
    displayPB.unload(CLEANER_SCREEN_MARGIN_X, CLEANER_SCREEN_MARGIN_Y, CLEANER_SCREEN_WIDTH, CLEANER_SCREEN_HEIGHT, SECOND_SAFETY_TIMEOUT);

    current_screen = flags;
  }
  else if (current_screen == flags){
    display.clearDisplay();
    display.drawXBitmap( 0, 0, epd_bitmap_flag, 128, 64, WHITE);
    display.display();
  }
  else if (current_screen == modality && selected == sumo){
    /*if (!Ps3.isConnected()) {
      display.clearDisplay();
      display.drawXBitmap( 0, 0, bitmap_screens[selected], 128, 64, WHITE);
      display.display();

      delay(250);

      display.clearDisplay();
      display.display();

      delay(250);
    } else {*/
      display.clearDisplay();
      display.drawXBitmap( 0, 0, bitmap_screens[selected], 128, 64, WHITE);
      display.display();
    //}
  }
  else if (current_screen == modality && selected == sprinter){
    display.clearDisplay();
    display.drawXBitmap( 0, 0, bitmap_screens[selected], 128, 64, WHITE);
    display.display();
  }

  UpdateScreenStatus();

  if (current_screen == selection){  
    motors.StayStill();
  }
}

void StartAreaCleanerSection() {
  
  // Area cleaner trigger
  if (current_screen == flags && selected == areaCleaner){}

}

#define PIN_LED 23

void setup(){    
  SerialBT.begin("Alita");

  pinMode(PIN_LED, OUTPUT);
  digitalWrite(PIN_LED, HIGH);

  // Begin display connection
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {    
    Serial.println(F("SSD1306 allocation failed"));
    for(;;);  
  }

  u8g2_for_adafruit_gfx.begin(display); // Begins u8g2 for gfx library

  pinMode(PIN_SELECT, INPUT_PULLUP);
  pinMode(PIN_DOWN, INPUT_PULLUP);

  // Displays team logo
  display.clearDisplay();
  display.drawBitmap( 0, 0, bitmap_alita, 128, 64, WHITE); // Prints teams logo
  display.display();

  delay(3000);
}

void loop() {
  DisplayMenu();
  StartAreaCleanerSection();    
} 
