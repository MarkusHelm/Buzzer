#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define PIN_NEO_PIXEL 10
#define NUM_PIXELS 12

Adafruit_NeoPixel NeoPixel(NUM_PIXELS, PIN_NEO_PIXEL, NEO_RGB + NEO_KHZ800);

enum AnimationState
{
  STOP,
  IDLE,
  COUNTDOWN,
  TIMEOUT,
  RECONNECT
};

AnimationState currAnimation = RECONNECT;

double maxIllumination = 0.5;
int position = 0;

void setup()
{
  NeoPixel.begin();
}

void loop()
{
  NeoPixel.clear(); // set all pixel colors to 'off'. It only takes effect if pixels.show() is called

  switch (currAnimation)
  {
  case STOP:
    NeoPixel.clear();
    break;

  case IDLE:
    break;

  case COUNTDOWN:
    break;

  case TIMEOUT:
    break;

  case RECONNECT:
    for (int j = 0; j < NUM_PIXELS; j++)
    {
      for (int i = NUM_PIXELS; i >= 0; i--)
      {
        int pixelIndex = (i + j) % NUM_PIXELS;                                // Damit die Animation einen Kreis bildet
        int brightness = map(i, 0, NUM_PIXELS, 255 * maxIllumination, 0);     // Helligkeit abhängig von der Position
        NeoPixel.setPixelColor(pixelIndex, NeoPixel.Color(0, 0, brightness)); // Nur Grün, Helligkeit variiert
      }
      NeoPixel.show();
      delay(100);       // Ändere diesen Wert, um die Geschwindigkeit der Animation anzupassen
      NeoPixel.clear(); // Lösche den Ring für die nächste Iteration
    }
    break;
  }

  NeoPixel.show();
}