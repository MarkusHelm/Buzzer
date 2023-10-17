#include <Arduino.h>
#include <Adafruit_NeoPixel.h>

#define PIN_NEO_PIXEL 10
#define NUM_PIXELS 12

Adafruit_NeoPixel NeoPixel(NUM_PIXELS, PIN_NEO_PIXEL, NEO_GRB + NEO_KHZ800);

enum AnimationState
{
  STOP,
  IDLE,
  FLASHING,
  COUNTDOWN,
  TIMEOUT,
  RECONNECT
};

// Global Variables
AnimationState currAnimation = COUNTDOWN;
unsigned long previousMillis = 0;
double maxIllumination = 1;

// Idle / Pulse
double pulseBrightness = floor((255 * maxIllumination + 255 * maxIllumination * 0.1) / 2);
double pulseDirection = maxIllumination;

// Flashing / Flashing
boolean flashingState = true;

// Countdown / Clock
int leftPixels = NUM_PIXELS;

// Reconnect / Circle
int cicleStartPosition = 0;

boolean isTimePassedFor(int minMillis);
void runIdleAnimation(int speed);
void runFlashingAnimation(int speed);
void runCountdownAnimation(double time);
void runReconnectAnimation(int speed);

void setup()
{
  NeoPixel.begin();
}

void loop()
{
  // Aktualisiere die Helligkeit basierend auf der Richtung

  switch (currAnimation)
  {
  case STOP:
    NeoPixel.clear();
    break;

  case IDLE:
    runIdleAnimation(10);
    break;

  case FLASHING:
    runFlashingAnimation(50);
    break;

  case COUNTDOWN:
    runCountdownAnimation(10.0);
    break;

  case TIMEOUT:
    NeoPixel.fill(NeoPixel.Color(5, 0, 0), 0, NUM_PIXELS);
    break;

  case RECONNECT:
    runReconnectAnimation(100);
    break;
  }

  NeoPixel.show();
}

// Millis Time Management
boolean isTimePassedFor(int minMillis)
{
  unsigned long currentMillis = millis();

  if (currentMillis - previousMillis >= minMillis)
  {
    previousMillis = currentMillis;

    return true;
  }

  return false;
}

// Animations
void runIdleAnimation(int speed)
{
  if (isTimePassedFor(speed))
  {
    pulseBrightness += pulseDirection;

    // Ändere die Richtung, wenn die Helligkeit ihre Grenzen erreicht
    if (pulseBrightness <= 255 * maxIllumination * 0.1 || pulseBrightness >= 255 * maxIllumination)
    {
      pulseDirection *= -1;
    }

    NeoPixel.clear();
    NeoPixel.fill(NeoPixel.Color((int)pulseBrightness, (int)pulseBrightness, (int)pulseBrightness), 0, NUM_PIXELS);
  }
}

void runFlashingAnimation(int speed)
{
  if (flashingState)
  {
    NeoPixel.fill(NeoPixel.Color((int)255 * maxIllumination, (int)255 * maxIllumination, (int)255 * maxIllumination), 0, NUM_PIXELS);
  }
  else
  {
    NeoPixel.clear();
  }

  if (isTimePassedFor(speed))
  {
    flashingState = !flashingState;
  }
}

void runCountdownAnimation(double time)
{
  int msBetweenAnimation = (int)((time * 1000) / NUM_PIXELS);

  NeoPixel.clear();
  NeoPixel.fill(NeoPixel.Color(0, (int)255 * maxIllumination, 0), NUM_PIXELS - leftPixels, NUM_PIXELS);

  if (isTimePassedFor(msBetweenAnimation))
  {
    if (leftPixels != 0)
    {
      leftPixels--;
    }
  }
}

void runReconnectAnimation(int speed)
{
  if (isTimePassedFor(speed))
  {
    NeoPixel.clear();

    for (int i = 0; i < NUM_PIXELS; i++)
    {
      int pixelIndex = (i + cicleStartPosition) % NUM_PIXELS;
      int loadingBrightness = map(i, 0, NUM_PIXELS, 0, 255 * maxIllumination);
      NeoPixel.setPixelColor(pixelIndex, NeoPixel.Color(0, 0, loadingBrightness));
    }

    cicleStartPosition++;

    if (cicleStartPosition > NUM_PIXELS - 1)
    {
      cicleStartPosition = 0;
    }
  }
}