#include <Arduino.h>
// LED Ring
#include <Adafruit_NeoPixel.h>
// Wifi
#include <esp_now.h>
#include <WiFi.h>

// Pin Setup
#define PIN_BTN D9
#define PIN_NEO_PIXEL D10
#define PIN_BAT A0
#define NUM_PIXELS 12

bool lastBtnState = HIGH;

Adafruit_NeoPixel NeoPixel(NUM_PIXELS, PIN_NEO_PIXEL, NEO_GRB + NEO_KHZ800);

uint8_t broadcastAddress[] = {0x34, 0x85, 0x18, 0x91, 0xC9, 0x48}; // Controller

typedef struct buzzerMsg
{
  int state;
} buzzerMsg;

typedef struct controllerMsg
{
  uint8_t r;
  uint8_t g;
  uint8_t b;
} controllerMsg;

esp_now_peer_info_t peerInfo;

float getBatteryVoltage(void)
{
  uint32_t Vbatt = 0;
  for (int i = 0; i < 16; i++)
  {
    Vbatt = Vbatt + analogReadMilliVolts(PIN_BAT); // ADC with correction
  }
  float Vbattf = 2 * Vbatt / 16 / 1000.0; // attenuation ratio 1/2, mV --> V
  return Vbattf;
}

void setColor(uint8_t r, uint8_t g, uint8_t b)
{
  const uint32_t color = NeoPixel.Color(r, g, b);
  NeoPixel.fill(color, 0, NUM_PIXELS);
  NeoPixel.show();
}

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
  controllerMsg packet;
  memcpy(&packet, incomingData, sizeof(packet));
  setColor(packet.r, packet.g, packet.b);
}

void setup()
{
  pinMode(PIN_BTN, INPUT_PULLUP);
  pinMode(PIN_BAT, INPUT);

  NeoPixel.begin();
  NeoPixel.setBrightness(25);
  setColor(0, 0, 10);

  Serial.begin(115200);

  // ESP-NOW Setup
  WiFi.mode(WIFI_STA);
  esp_now_init();
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);
}

void loop()
{
  const bool btnState = digitalRead(PIN_BTN);

  if (btnState != lastBtnState)
  {
    buzzerMsg packet;
    packet.state = btnState;
    esp_now_send(broadcastAddress, (uint8_t *)&packet, sizeof(packet));
  }

  lastBtnState = btnState;
}