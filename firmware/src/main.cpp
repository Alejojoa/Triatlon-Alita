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



//---------------------------------------------------------------------------------------------   

//Para los motores usamos una lirería propia
//motors.MoveForwards (MID_SPEED , MID_SPEED) se indica la dirección y el PWM de cada rueda
//motors.StayStill() baja a 0 los PWM de cada rueda
//motors.Brake() pone en corto los drivers

int datoBT;

#define QRE_BLACK          3900
#define SHARP_ATAQUE       1000
#define SET_BACKWARDS_TIME  350 

#define LOW_SPEED   100
#define MID_SPEED   150
#define FULL_SPEED  200

#define INCREASE_SPEED        10 
#define INCREASE_SPEED_DELAY 500

bool offRoad;



int sharp_left;
int sharp_right;
int sharp_front_right;
int sharp_front;
int sharp_front_left;

int any_qre_front;
bool qre_front;
bool qre_back;

int signal_input;

CD74HC4067 my_mux(4, 25, 33, 32); // s0, s1, s2, s3

#define PIN_SIG 34

void ReadCleanerSensors() {   //Lectura del multiplexor

  qre_front = false;
  qre_back = false;

  for (int x = 0; x < 15; x++) {
    
    my_mux.channel(x);
  
    signal_input = analogRead(PIN_SIG);

    switch (x) {

      case 0: //Tuvimos problemas con la lectura de los qre laterales, entonces empezamos a usar la regleta completa
      case 1: //Es temporal igual, en el próximo robot esto sería corregido
      case 2:
      case 3:
      case 4:
      case 5:
      case 6:
      case 7: {
        any_qre_front = signal_input;
        if (any_qre_front > QRE_BLACK) {
          qre_front = true;
        }
        break;
      }
      case 8: {
        sharp_right = signal_input;
        break;
      }
      case 9: {
        sharp_front_right = signal_input;
        break;
      }
      case 10: {
        sharp_front = signal_input;
        break;
      }
      case 11: {
        sharp_front_left = signal_input; 
        break;
      }
      case 12: { 
        sharp_left = signal_input;
        break;
      } 
      case 14: {
        if (signal_input > QRE_BLACK) {
          qre_back = true;
        }
        break;
      }
    } 
  }

  if (qre_front || qre_back) {    //Para trabajar más cómodo, lo uní a la lectura de los sensores
    offRoad = true;
  } 
  else {
    offRoad = false;
  }
}




void Out() {  //Este void se encarga de vovler a la pista cuando offRoad = true;

  ReadCleanerSensors();

  unsigned long time;


  if (qre_front) { 
    motors.Brake();
    delay (200);
     
    time = millis();

    while (millis() < time + SET_BACKWARDS_TIME) {
      motors.MoveBackwards (MID_SPEED , MID_SPEED);
      delay (10);
    }

    ReadCleanerSensors();

    SerialBT.println ("FRONT");
  }

  if (qre_back) {
    
    time = millis();

    while (millis() < time + SET_BACKWARDS_TIME) {
      motors.MoveBackwards (FULL_SPEED , FULL_SPEED);
      delay (10);
    }

    ReadCleanerSensors();

    SerialBT.println ("BACK");
  }
}





void SearchObject() {   //Para encotrar objetos cuando ningúno de los sharps ve nada y no registró nada al avanzar

  ReadCleanerSensors();
  long time_limit;  
  long time_circle;

  time_circle = millis();

  while (millis() < time_circle + 6000) {  //La idea es que de un círculo completo moviendose de a poco
  //Por limitaciones de los drivers no puedo hacerlos girar a un PWM menor a 100, por eso los ciclos de acelerar y frenar

    for (int i=0 ; i<50 ; i++) {
      time_limit = millis();

      while (millis() < time_limit + 250) {
        motors.StayStill(); 
        delay (10);
      }
      while (millis() < time_limit + 200) { 
        motors.TurnRight (0 , FULL_SPEED);
        delay (10);
      }

      ReadCleanerSensors();

      if (sharp_front > SHARP_ATAQUE) {
        break;
      }
    }
  }
}


long time_totals;

long time_found_left;
long time_found_right;
int total_found_left;
int total_found_right;
//Estos "times" son para tomar el tiempo transcurrido desde que vío algún objeto al avanzar y así retroceder hasta quedar a la par

bool state_found_left;
bool state_found_right;
bool state_offRoad;


void StartAreaCleanerSection() {

  ReadCleanerSensors();

  switch (offRoad) {  

    case true: {
      Out();
      break;
    }

    case false: {

      SerialBT.println (time_totals);

      if (total_found_left < total_found_right) {            //Al volver offRoad a false, que cumpla con el último resultado
        while (millis() < time_totals + total_found_left) {  //¿Podría no cumplirse por haber renovado todas las variables al pasar de un case a otro?
          motors.MoveBackwards (MID_SPEED , MID_SPEED); 
        }
        ReadCleanerSensors();
      }
      if (total_found_left > total_found_right) {
        while (millis() < time_totals + total_found_right) {
          motors.MoveBackwards (MID_SPEED , MID_SPEED);
          delay (10);
        }
        ReadCleanerSensors();
      }

      if ((sharp_front | sharp_front_left | sharp_front_right) > SHARP_ATAQUE) {

        motors.MoveForward (255 , 255);
        delay (70); //Como los motores de clavaban (Los drivers estaban en las últimas) le daba un golpe antes de avanzar

        for (int i=LOW_SPEED ; i<=MID_SPEED ; i+=INCREASE_SPEED) { //Para acelerar paulatinamente
          
          motors.MoveForward (i , i);
          delay (INCREASE_SPEED_DELAY);

          ReadCleanerSensors();

          if (offRoad) {
            break;
          }

          delay (10);
        }


        if (sharp_left > SHARP_ATAQUE) {
          state_found_left = true;
          time_found_left = millis();
        }
        if (sharp_right > SHARP_ATAQUE) {
          state_found_right = true;
          time_found_right = millis();
        } //Para comenzar a tomar el tiempo si ve algo a los costados mientras ataca

        ReadCleanerSensors();
      } 
      
      else {
        SearchObject();
      }

      
      if (offRoad) {
        time_totals = millis();
        
        if (state_found_left) { //Para sacar el total del tiempo transcurrido
          total_found_left = millis() - time_found_left;
        }
        if (state_found_right) {
          total_found_right = millis() - time_found_right;
        }
      }
      //Esta es la parte a la que más le desconfío. Una vez funcionó, pero no más
      //A lo que le temo es que cuando offRoad = true, pase directo al void Out() antes de sacar los tiempos totales
      //En caso de que esto pase, ¿Debería solucionarse pasando este ultimo bloque en el case true:?
      break;
    }
  }
}




//---------------------------------------------------------------------------------------------

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
  if (current_screen == flags && selected == areaCleaner){
    StartAreaCleanerSection(); 
  }
} 