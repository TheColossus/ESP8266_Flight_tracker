#include <espnow.h>
#include <ESP8266WiFi.h>
#include "flight_tracker_reciever.h"

void setup() {
    Serial.begin(921600);
    WiFi.mode (WIFI_STA);
    if (esp_now_init() != 0) {
      Serial.println("ESP-NOW Init Failed!");
  } else {
      Serial.println("ESP-NOW Init Success!");
  }

  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);
  esp_now_register_recv_cb(OnDataRecv);

}
void loop() {
    if (newDataAvailable) {
        newDataAvailable = false;
        for (int i = 0; i < 3; ++i) {
            ScrollTextLoop(incoming_info.callsign, incoming_info.model, 35);
        }
    }
}
