#include "flight_tracker_transmitter.h"
#include <ESP8266WiFi.h>
#include <espnow.h>

uint8_t peerMAC[] = {0x68, 0xC6, 0x3A, 0xE6, 0xC8, 0x30};  

char prev_icao[7] = "";


void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus);

void setup() {
    Serial.begin(921600);
    delay(3000);

    Serial.print("Attempting Wifi connection.");
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    while (WiFi.status() != WL_CONNECTED) {
        Serial.print(".");
        delay(1000);
    }

    Serial.print("\nConnected to Wifi with IP: ");
    Serial.println(WiFi.localIP());
    get_token();
    WiFi.mode(WIFI_STA); // Required for ESP-NOW

    if (esp_now_init() != 0) {
        Serial.println("ESP-NOW Init Failed!");
    } else {
        Serial.println("ESP-NOW Init Success!");
    }

    esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
    esp_now_register_send_cb(OnDataSent);
    esp_now_add_peer(peerMAC, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);

}

void loop() {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    flight_info info;
    get_info(&info);
    if (info.icao24[0] != '\0' && strcmp(info.icao24, prev_icao)) {
        Serial.println(info.icao24);
        if (info.callsign[0] != '\0') {
            Serial.println(info.callsign);
            esp_now_send(peerMAC, (uint8_t *)&info, sizeof(info));
        }
        Serial.println(info.model);
        strcpy(prev_icao, info.icao24);
    }

    delay(5000);
}


void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {
  Serial.println(sendStatus == 0 ? "Delivery Success" : "Delivery Fail");
}