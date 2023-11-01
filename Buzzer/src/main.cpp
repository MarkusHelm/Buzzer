#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <Adafruit_NeoPixel.h>

// buzzer setup
#define BUZZER_ID "2"

// pin setup
#define PIN_BTN D9
#define PIN_NEO_PIXEL D10
#define PIN_BAT A0

// led setup
#define LED_PIXEL_COUNT 12
#define LED_STROBE_SPEED 50
// --------
uint8_t ledColor[3] = {0, 0, 255};
uint8_t ledBrightness = 50;
uint8_t ledMode = 0; // 0=static, 1=strobe
// --------

Adafruit_NeoPixel NeoPixel(LED_PIXEL_COUNT, PIN_NEO_PIXEL, NEO_GRB + NEO_KHZ800);

// wifi setup
#define WIFI_SSID "BuzzerNet"
#define WIFI_PASSWORD "Hallo1234"

// http setup
uint8_t httpLastUpdateAt = 0;

// controler setup
#define CONTROLLER_IP "192.168.4.1"
#define CONTROLLER_PORT 80

//  utility functions
float getBatteryVoltage()
{
  // imit battery voltage measurement
  uint32_t Vbatt = 0;

  // 16 measurements for better accuracy
  for (int i = 0; i < 16; i++)
  {
    // read battery voltage in mV
    Vbatt = Vbatt + analogReadMilliVolts(PIN_BAT);
  }

  // return calculate average (attention: ratio 1/2 and convert mV to V)
  return 2 * Vbatt / 16 / 1000.0;
}

// TODO: TITUS lesen
void showLED()
{
  NeoPixel.setBrightness(ledBrightness);
  switch (ledMode)
  {
  case 0: // static
    NeoPixel.fill(NeoPixel.Color(ledColor[0], ledColor[1], ledColor[2]), 0, LED_PIXEL_COUNT);
    break;
  case 1: // strobe
    if (millis() % (2 * LED_STROBE_SPEED) < LED_STROBE_SPEED)
      NeoPixel.fill(NeoPixel.Color(ledColor[0], ledColor[1], ledColor[2]), 0, LED_PIXEL_COUNT);
    else
      NeoPixel.fill(NeoPixel.Color(0, 0, 0), 0, LED_PIXEL_COUNT);
    break;
  default: // led off
    NeoPixel.fill(NeoPixel.Color(0, 0, 0), 0, LED_PIXEL_COUNT);
    break;
  }
  NeoPixel.show();
}
void connectWifi()
{
  // check if WiFi is not connected to reconnect (or connect for the first time)
  if (!WiFi.isConnected())
  {
    Serial.print("WiFi is not Connected.");
    Serial.print("Connecting to WiFi ...");

    // connect to WiFi
    while (WiFi.status() != WL_CONNECTED)
    {
      WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
      Serial.print(".");
      delay(500);
    }
    Serial.println("Connected successfully to the WiFi network!");
  }
}
void callController(boolean buttonPressed = false)
{
  // setup clients
  WiFiClient client;
  HTTPClient http;

  // send http request
  if (http.begin(client, "http://" + String(CONTROLLER_IP) + ":" + String(CONTROLLER_PORT) + "?buzzerId=" + String(BUZZER_ID) + "&buttonPressed=" + String(buttonPressed) + "&batteryVoltage=" + String(getBatteryVoltage()) + "&end"))
  {
    // get http code and payload
    int httpCode = http.GET();

    if (httpCode > 0 && httpCode == HTTP_CODE_OK)
    {
      // payload definition: color;brightness;mode; (e.g. 255,0,0;100;0;)
      String payload = http.getString();

      // split payload by ; and set payload split
      char *payloadSplit = strtok((char *)payload.c_str(), ";");

      // split first part of splitted payload by , and set color split
      char *colorSplit = strtok(payloadSplit, ",");

      // set color
      ledColor[0] = atoi(colorSplit);
      colorSplit = strtok(NULL, ",");
      ledColor[1] = atoi(colorSplit);
      colorSplit = strtok(NULL, ",");
      ledColor[2] = atoi(colorSplit);

      // set brightness
      payloadSplit = strtok(NULL, ";");

      // set mode
      payloadSplit = strtok(NULL, ";");
    }
    else
    {
      Serial.println("Error on HTTP request (httpCode: " + String(httpCode) + ")");
    }
    http.end();
  }

  // update last http update
  httpLastUpdateAt = millis();
}

// setup function
void setup()
{
  // setup pins
  pinMode(PIN_BTN, INPUT_PULLUP);
  pinMode(PIN_BAT, INPUT);

  // setup led
  NeoPixel.begin();
  showLED();

  // setup serial
  Serial.begin(115200);

  // call connect wifi to connect to wifi
  connectWifi();
}

// loop function
void loop()
{
  // show led
  showLED();
  // call connect wifi to reconnect if needed
  connectWifi();

  // check if last http update is older than 250ms and call controller
  if (millis() - httpLastUpdateAt > 250)
  {
    callController();
  }

  // if button press call controller
  if (digitalRead(PIN_BTN) == LOW)
  {
    callController(true);
  }
}