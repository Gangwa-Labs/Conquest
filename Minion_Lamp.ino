#include <ESP8266WiFi.h>
#include <espnow.h>
#include <FastLED.h>
#define NUM_LEDS 128
#define NUM_STAND 32
//GPIO6, GPIO7, GPIO8, GPIO9, GPIO10, and GPIO11 are all unavailable
#define DATA_PIN  2

CRGB leds[NUM_LEDS];


typedef struct struct_message {//matching data structure for mic side
  byte a = 1;
  byte b;
} struct_message;

struct_message myData;

DEFINE_GRADIENT_PALETTE( red_blue ) { //blue -> red
  0,   0,  0,  255,
  128,   255, 0, 0,
  255,   0, 0, 255
};
DEFINE_GRADIENT_PALETTE( green_purple ) { //green -> purple
  0,   0,  255,  0,
  128,   255, 0, 255,
  255,   0, 255, 0
};

bool lastSyncState = LOW;
bool syncSplitter = LOW;


int animationSpeed = 10;
CRGBPalette16 currentPalette;
uint8_t paletteIndex = 0;

void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.println(myData.a);
  Serial.println(myData.b);
  if (myData.a == 1) {
    currentPalette = RainbowColors_p;
    animationSpeed = 100;
  } else if (myData.a == 2) {
    currentPalette = red_blue;
  } else if (myData.a == 3) {
    currentPalette = green_purple;
  }
  if (myData.b == 1 & lastSyncState != 1) {
    paletteIndex = 0;
    syncSplitter = LOW;
    lastSyncState = 1;
    Serial.println("Palette 0ing");
  } else if (myData.b == 0 & lastSyncState != 0) {
    paletteIndex = 0;
    syncSplitter = HIGH;
    lastSyncState = 0;
    Serial.println("Palette going brazy");
  }
}

void setup() {
  // put your setup code here, to run once:
  FastLED.addLeds<WS2811, DATA_PIN>(leds, NUM_LEDS);
  Serial.begin(115200);
  FastLED.setBrightness(255);
  WiFi.mode(WIFI_STA);//sets wifi mode as a station
  // Init ESP-NOW
  if (esp_now_init() != 0) {
    Serial.println("Error initializing ESP-NOW");
    return;
  } else {
    Serial.println("sucess");
  }
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  // put your main code here, to run repeatedly:
  EVERY_N_MILLISECONDS(animationSpeed) {
    paletteIndex--;
  }
  EVERY_N_MILLISECONDS(25) {
    if (syncSplitter == LOW) {
      fill_palette(leds, NUM_LEDS, paletteIndex, 255 / NUM_LEDS, currentPalette, 255, LINEARBLEND);
    } else {
      fill_palette(leds, NUM_LEDS, paletteIndex, 255 / NUM_STAND, currentPalette, 255, LINEARBLEND);
      fill_palette(leds, NUM_LEDS, paletteIndex + 32, 255 / NUM_STAND, currentPalette, 255, LINEARBLEND);
      fill_palette(leds, NUM_LEDS, paletteIndex + 64, 255 / NUM_STAND, currentPalette, 255, LINEARBLEND);
      fill_palette(leds, NUM_LEDS, paletteIndex + 96, 255 / NUM_STAND, currentPalette, 255, LINEARBLEND);
    }
    FastLED.show();
  }
}
