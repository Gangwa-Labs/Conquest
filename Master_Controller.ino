#include <esp_now.h>
#include <WiFi.h>

const int buttonPinLeft = 27;
const int buttonPinRight = 33;
const int syncPin = 34;

uint8_t broadcastAddress1[] = {0xc4, 0x5b, 0xbe, 0x71, 0x5d, 0xaa};
uint8_t broadcastAddress2[] = {0x30, 0x83, 0x98, 0x82, 0xe8, 0x12};

int buttonStateLeft = 0;
int buttonStateRight = 0;
int buttonStateSync = 0;
int preStateLeft = 0;
int preStateRight = 0;
int preStateSync = 0;
bool buttonPressed = LOW;

unsigned long previousMillis = 0;
const long interval = 50;

typedef struct struct_message {
  byte a = 1;
  byte b; //low is split animations all 32 synced, HIGH is full animation across 128
} struct_message;
struct_message myData;

void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t sendStatus) {
  char macStr[18];
  Serial.print("Packet to:");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print(macStr);
  Serial.print(" send status: ");
  Serial.println(sendStatus == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void dataCheckSend(bool side) {
  if (side == HIGH) { //high means left low means right
    myData.a--; //Left side commands
    preStateLeft = 1;
  } else {
    myData.a++; //Right side commands
    preStateRight = 1;
  }

  if (myData.a > 3) {
    myData.a = 1;
  }
  if (myData.a  == 0) {
    myData.a = 3;
  }
  buttonPressed = HIGH;
}

void setup() {
  // put your setup code here, to run once:
  pinMode(buttonPinLeft, INPUT);
  pinMode(buttonPinRight, INPUT);
  pinMode(syncPin, INPUT);
  //Init Serial USB
  Serial.begin(115200);
  //Enable wifi mode
  WiFi.mode(WIFI_STA);
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  //establish the role of the master for esp_now
  esp_now_register_send_cb(OnDataSent);
  //Register a peer
  esp_now_peer_info_t peerInfo;
  peerInfo.channel = 0;
  peerInfo.encrypt = false;
  // register first peer
  memcpy(peerInfo.peer_addr, broadcastAddress1, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
  memcpy(peerInfo.peer_addr, broadcastAddress2, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK) {
    Serial.println("Failed to add peer");
    return;
  }
}

void loop() {
  buttonStateLeft = digitalRead(buttonPinLeft);
  buttonStateRight = digitalRead(buttonPinRight);
  buttonStateSync = digitalRead(syncPin);
  if (buttonStateLeft == HIGH & preStateLeft == 0) {
    Serial.println("left");
    dataCheckSend(HIGH);
  } else if (buttonStateLeft == LOW) {
    preStateLeft = 0;
  }

  if (buttonStateRight == HIGH & preStateRight == 0) {
    Serial.println("Right");
    dataCheckSend(LOW);
  } else if (buttonStateRight == LOW) {
    preStateRight = 0;
  }

  if (buttonStateSync == HIGH & preStateSync == 0) {
    myData.b++;
    if (myData.b > 1) {
      myData.b = 0;
    }
    preStateSync = 1;
    buttonPressed = HIGH;
  } else if (buttonStateSync == LOW) {
    preStateSync = 0;
  }

  //send time
  if (buttonPressed == HIGH) {
    esp_err_t result = esp_now_send(0, (uint8_t *) &myData, sizeof(myData));
    if (result == ESP_OK) {
      Serial.println("Sent with success");
    }
    else {
      Serial.println("Error sending the data");
    }
    Serial.print("Animation Index: ");
    Serial.println(myData.a);
    Serial.print("Sync State: ");
    Serial.println(myData.b);
    buttonPressed = LOW;
  }
  delay(0);
}
