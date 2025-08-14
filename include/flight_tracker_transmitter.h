#pragma once

#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include "credentials.h"

#define token_buffer_size 2048


typedef struct flight_info {
    char icao24[7];
    char callsign[9];
    char model[128];
} flight_info;

void get_info(flight_info* info);
void get_token();
void to_uppercase(char* upper_str, const char* str);

String token;
// char token_buffer[token_buffer_size];

void get_info(flight_info* info) {
    Serial.println("getting info");
    WiFiClientSecure client;
    JsonDocument data_json;
    HTTPClient http;
    client.setInsecure();
    http.useHTTP10(true);
    http.begin(client, "https://opensky-network.org/api/states/all?lamin=50.83&lomin=-114.05&lamax=50.87&lomax=-113.97");
    String payload = "Bearer " + token;
    // strcat(payload, token_buffer);
    http.addHeader("Authorization", payload, false, true);
    Serial.println("get");  
    int http_response_code = http.GET();
    Serial.print("Data fetch response code = ");
    Serial.println(http_response_code);
    if (http_response_code < 0) {
        Serial.println("Request failed");
    } else if (http_response_code == 401) { //Get new token
        http.end();
        get_token();
        http.begin(client, "https://opensky-network.org/api/states/all?lamin=50.83&lomin=-114.05&lamax=50.87&lomax=-113.97");
        http.useHTTP10(true);
        payload = "Bearer " + token;
        http.addHeader("Authorization", payload, false, true);
        http_response_code = http.GET();
    }
    Serial.println("deserialize");
    DeserializationError error = deserializeJson(data_json, http.getStream());
    http.end();
    if (error) {
        Serial.print("Data deserialization failed: ");
        Serial.println(error.c_str());
        return;
    } else {
        JsonArray states = data_json["states"].as<JsonArray>();
        if (states.size() == 0) {
            Serial.println("States array empty");
            info->icao24[0] = '\0';
            info->callsign[0] = '\0';
            return;
        }

        JsonArray first_state = states[0].as<JsonArray>();
        if (first_state.size() == 0) {
            Serial.println("First state vector empty");
            info->icao24[0] = '\0';
            info->callsign[0] = '\0';
            return;
        }

        const char* icao24 = first_state[0]; // index 0 is icao24 
        if (icao24 == nullptr) {
            Serial.println("icao24 is null");
            info->icao24[0] = '\0';
            return;
        }

        const char* callsign = first_state[1]; // index 1 is callsign 
        if (icao24 == nullptr) {
            Serial.println("callsign is null");
            info->callsign[0] = '\0';
            return;
        }
        strncpy(info->callsign, callsign, 8); //Callsigns are 8 characters
        strncpy(info->icao24, icao24, 6); //icao24's are 6 characters
        info->callsign[8] = '\0';
        info->icao24[6] = '\0';
        
        char url[64] = "http://cfapi.schaturvedi.workers.dev/api/";
        char icao_uppercase[7];
        to_uppercase(icao_uppercase, icao24);
        icao_uppercase[6] = '\0';
        strncat(url, icao_uppercase, 6);
        Serial.println(url);
        HTTPClient http2;
        WiFiClient client2;
        http2.begin(client2, url);
        http_response_code = http2.GET();
        if (http_response_code < 0) {
            Serial.println("Model Request failed");
        } else {
            WiFiClient* stream = http2.getStreamPtr();
            if (http2.getSize() > 0 && stream->available()) {
                size_t len = http2.getSize();
                size_t read_len = stream->readBytes(info->model, min(len, sizeof(info->model) - 1));
                info->model[read_len] = '\0';
            }
        http2.end();
        }
    }
}

void get_token() {
    Serial.println("Getting token");
    WiFiClientSecure client;
    JsonDocument token_json;
    HTTPClient http;
    client.setInsecure();    
    http.useHTTP10(true);
    http.begin(client, "https://auth.opensky-network.org/auth/realms/opensky-network/protocol/openid-connect/token");
    http.addHeader("Content-Type", "application/x-www-form-urlencoded");
    String post_data;
    post_data.reserve(256);  // Preallocate enough space to avoid fragmentation (using String is unavoidable here)
    post_data = "grant_type=client_credentials&client_id=";
    post_data += CLIENT_ID;
    post_data += "&client_secret=";
    post_data += CLIENT_SECRET;
    Serial.println("post");
    int http_response_code = http.POST(post_data);
    if (http_response_code < 0) {
        Serial.println("Request failed");
    }
    DeserializationError error = deserializeJson(token_json, http.getStream());
    http.end();
    if (error) {
    Serial.print("Token deserialization failed: ");
    Serial.println(error.c_str());
    return ;
    } else {
        token = token_json["access_token"].as<String>();
    }
}

void to_uppercase(char* upper_str, const char* str) {
    while (*str) {
        *upper_str = toupper(*str);
        str++;
        upper_str++;
    }
}