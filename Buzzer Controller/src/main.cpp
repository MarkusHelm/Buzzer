#include <Arduino.h>
#include <WiFi.h>
#include <WiFiAP.h>
#include <HTTPClient.h>
#include <U8x8lib.h> // OLED display
#include <Wire.h>    // required by U8x8lib

// pin setup
#define PIN_SW D8
#define PIN_BTN_1 D3
#define PIN_BTN_2 D2
#define PIN_BTN_3 D1
#define PIN_BAT A0

// OLED display setup
U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(SCL, SDA, U8X8_PIN_NONE);

// wifi setup
#define WIFI_SSID "BuzzerNet"
#define WIFI_PASSWORD "Hallo1234"
WiFiServer server;

// controler setup
IPAddress controllerIP(192, 168, 4, 1);
IPAddress gateway(192, 168, 4, 1);
IPAddress subnet(255, 255, 255, 0);
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

void setupWifiAP()
{
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(controllerIP, gateway, subnet);
  if (!WiFi.softAPConfig(controllerIP, gateway, subnet))
  {
    Serial.println("AP Config failed.");
    return;
  }
  WiFi.softAP(WIFI_SSID, WIFI_PASSWORD);

  server = WiFiServer(CONTROLLER_PORT);
}
void runServer()
{
  WiFiClient client = server.available();
  if (client)
  {
    Serial.println("New client");
    while (client.connected())
    {
      if (client.available())
      {
        // get buzzerId, buttonPressed and batteryVoltage via query string
        String buzzerId = "";
        String buttonPressed = "";
        String batteryVoltage = "";

        // read first line of the request
        String req = client.readStringUntil('\r');

        Serial.println("request: " + req); // e.g. "GET /?buzzerId=1&buttonPressed=1&batteryVoltage=4.2&end HTTP/1.1

        // parse query string
        int buzzerIdStart = req.indexOf("buzzerId=");
        int buzzerIdEnd = req.indexOf("&buttonPressed=");
        buzzerId = req.substring(buzzerIdStart + 9, buzzerIdEnd);

        int buttonPressedStart = req.indexOf("&buttonPressed=");
        int buttonPressedEnd = req.indexOf("&batteryVoltage=");
        buttonPressed = req.substring(buttonPressedStart + 15, buttonPressedEnd);

        int batteryVoltageStart = req.indexOf("&batteryVoltage=");
        int batteryVoltageEnd = req.indexOf("&end");
        batteryVoltage = req.substring(batteryVoltageStart + 16, batteryVoltageEnd);

        // create payload definition: color;brightness;mode; (e.g. 255,0,0;100;0;)
        String payload = "";

        // set color
        payload += "255,0,0;";

        // set brightness
        payload += "100;";

        // set mode
        payload += "0;";

        // send payload
        client.println(payload);
        break;
      }
    }
    client.stop();
    Serial.println("Client disconnected");
  }
}

// setup method
void setup()
{
  // OLED setup
  u8x8.begin();
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.setCursor(0, 0);
  u8x8.println("Starting up...");

  // pin configuration
  pinMode(PIN_SW, INPUT_PULLUP);
  pinMode(PIN_BTN_1, INPUT_PULLUP);
  pinMode(PIN_BTN_2, INPUT_PULLUP);
  pinMode(PIN_BTN_3, INPUT_PULLUP);
  pinMode(PIN_BAT, INPUT);

  // setup wifi
  setupWifiAP();
  u8x8.println("Wifi setup done");
}

// loop method
void loop()
{
  runServer();
}