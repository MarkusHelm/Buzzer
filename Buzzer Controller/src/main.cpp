#include <Arduino.h>
// OLED
#include <U8x8lib.h>
#include <Wire.h>
// Wifi
#include <esp_now.h>
#include <WiFi.h>

// Pin Setup
#define PIN_SW D8
#define PIN_BTN_1 D3
#define PIN_BTN_2 D2
#define PIN_BTN_3 D1
#define PIN_BAT A0

// OLED Display Setup
U8X8_SSD1306_128X64_NONAME_HW_I2C u8x8(SCL, SDA, U8X8_PIN_NONE);

int buzzer1state = 0;

// Buzzer 1 MAC  34:85:18:05:BF:10
uint8_t broadcastAddress[] = {0x34, 0x85, 0x18, 0x05, 0xBF, 0x10};

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

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status)
{
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void OnDataRecv(const uint8_t *mac, const uint8_t *incomingData, int len)
{
  buzzerMsg packet;
  memcpy(&packet, incomingData, sizeof(packet));
  buzzer1state = packet.state;
  controllerMsg cmsg;
  cmsg.r = 255 * buzzer1state;
  cmsg.g = 255 * !buzzer1state;
  cmsg.b = 0;
  esp_now_send(broadcastAddress, (uint8_t *)&cmsg, sizeof(cmsg));
}

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

void setup(void)
{
  // OLED Setup
  u8x8.begin();
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.setCursor(0, 0);
  u8x8.println("Booting...");

  Serial.begin(115200);

  // Pin configuration
  pinMode(PIN_SW, INPUT_PULLUP);
  pinMode(PIN_BTN_1, INPUT_PULLUP);
  pinMode(PIN_BTN_2, INPUT_PULLUP);
  pinMode(PIN_BTN_3, INPUT_PULLUP);
  pinMode(PIN_BAT, INPUT);

  WiFi.mode(WIFI_STA);
  esp_now_init();
  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  esp_now_add_peer(&peerInfo);

  u8x8.clear();
}

void loop(void)
{
  u8x8.setFont(u8x8_font_chroma48medium8_r);
  u8x8.setCursor(0, 0);
  u8x8.println("SW: " + String(!digitalRead(PIN_SW)));
  u8x8.println("B1: " + String(!digitalRead(PIN_BTN_1)));
  u8x8.println("B2: " + String(!digitalRead(PIN_BTN_2)));
  u8x8.println("B3: " + String(!digitalRead(PIN_BTN_3)));
  u8x8.println("BAT: " + String(getBatteryVoltage(), 1));
  u8x8.println("BUZZER 1: " + String(!buzzer1state));
}
