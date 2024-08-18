#ifndef PROGRESS_BARS_H
#define PROGRESS_BARS_H

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

class ProgressBar {
    private:
        Adafruit_SSD1306 &_display;
  
    public:
        ProgressBar(Adafruit_SSD1306 &display);
        void load(int x, int y, int width, int height, unsigned long duration);
        void unload(int x, int y, int width, int height, unsigned long duration);
};

#endif