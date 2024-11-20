#include "progress_bars.h"

ProgressBar::ProgressBar(Adafruit_SH1106 &display) : _display(display) {}

void ProgressBar::load(int x, int y, int width, int height, unsigned long duration)
{
  unsigned long startTime = millis();

  while (millis() - startTime <= duration)
  {
    _display.clearDisplay();
    _display.drawXBitmap(0, 0, bitmap_screens[selected], 128, 64, WHITE);

    unsigned long currentTime = millis() - startTime;
    int progress = map(currentTime, 0, duration, 0, width);

    _display.fillRect(x, y, progress, height, BLACK);
    _display.display();
  }
}

void ProgressBar::unload(int x, int y, int width, int height, unsigned long duration)
{
  unsigned long startTime = millis();

  while (millis() - startTime <= duration)
  {
    _display.clearDisplay();
    _display.drawXBitmap(0, 0, bitmap_screens[selected], 128, 64, WHITE);

    unsigned long currentTime = millis() - startTime;
    int progress = map(currentTime, 0, duration, 0, width + 1);

    _display.fillRect((progress + x), y, (width - progress), height, BLACK);
    _display.display();
  }
}