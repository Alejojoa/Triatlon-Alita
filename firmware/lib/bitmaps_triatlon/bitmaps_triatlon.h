#ifndef BITMAPS_TRIATLON_H
#define BITMAPS_TRIATLON_H

#include <Arduino.h>

// Bitmap declaration
extern const unsigned char bitmap_screen_sprinter[] PROGMEM;
extern const unsigned char bitmap_screen_sumo[] PROGMEM;
extern const unsigned char bitmap_screen_area[] PROGMEM;

extern const unsigned char bitmap_icon_area[] PROGMEM;
extern const unsigned char bitmap_icon_sumo[] PROGMEM;
extern const unsigned char bitmap_icon_sprinter[] PROGMEM;

extern const unsigned char epd_bitmap_selected_background[] PROGMEM;
extern const unsigned char epd_bitmap_flag[] PROGMEM;
extern const unsigned char bitmap_alita[] PROGMEM;
extern const unsigned char bitmap_calibration[] PROGMEM;

// Bitmap arrays
extern const unsigned char* bitmap_screens[3];
extern const unsigned char* bitmap_icons[3];

// Constants 
#define NUM_ITEMS 3
#define MAX_ITEM_LENGTH 20

#define PIN_SELECT 18
#define PIN_DOWN 5

// Menu order
extern char menu_items[NUM_ITEMS][MAX_ITEM_LENGTH];

// Modality and flag screens 
enum screens {
    selection,
    modality,
    calibration,
    flags,
};

enum modalities {
    areaCleaner,
    sprinter,
    sumo,
};

// Global variables
extern modalities selected;
extern screens current_screen;
extern int next;
extern int previous;

// Menu system
void UpdateScreenStatus();

#endif