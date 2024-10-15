#ifndef PROGRESS_BARS_H
#define PROGRESS_BARS_H

#include <Adafruit_GFX.h>
#include <Adafruit_SH1106.h>
#include <bitmaps_triatlon.h>

class ProgressBar {
    private:
        Adafruit_SH1106 &_display;
  
    public:
        ProgressBar(Adafruit_SH1106 &display);
        void load(int x, int y, int width, int height, unsigned long duration);
        void unload(int x, int y, int width, int height, unsigned long duration);
};

#endif