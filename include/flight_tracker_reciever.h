#pragma once

#include <ArduinoJson.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <NeoPixelBrightnessBus.h>
/**************************************************************************/

/*
 * Definition of the LED matrix layout 
 */
constexpr uint8_t PanelWidth = 32;
constexpr uint8_t PanelHeight = 8;
constexpr uint16_t PixelCount = PanelWidth * PanelHeight;

NeoTopology<ColumnMajorAlternating180Layout> topo (PanelWidth, PanelHeight);
NeoPixelBrightnessBus<NeoGrbFeature, Neo800KbpsMethod> strip (PixelCount);

/**************************************************************************/

const uint8_t font5x7[][5] PROGMEM = {
    {0x7E, 0x09, 0x09, 0x7E, 0x00},  // A
    {0x7F, 0x49, 0x49, 0x36, 0x00},  // B
    {0x3E, 0x41, 0x41, 0x22, 0x00},  // C
    {0x7F, 0x41, 0x41, 0x3E, 0x00},  // D
    {0x7F, 0x49, 0x49, 0x41, 0x00},  // E
    {0x7F, 0x09, 0x09, 0x01, 0x00},  // F
    {0x3E, 0x41, 0x51, 0x32, 0x00},  // G
    {0x7F, 0x08, 0x08, 0x7F, 0x00},  // H
    {0x41, 0x7F, 0x41, 0x00, 0x00},  // I
    {0x20, 0x40, 0x41, 0x3F, 0x00},  // J
    {0x7F, 0x08, 0x14, 0x63, 0x00},  // K
    {0x7F, 0x40, 0x40, 0x40, 0x40},  // L
    {0x7F, 0x02, 0x04, 0x02, 0x7F},  // M
    {0x7F, 0x06, 0x18, 0x7F, 0x00},  // N
    {0x3E, 0x41, 0x41, 0x3E, 0x00},  // O
    {0x7F, 0x09, 0x09, 0x06, 0x00},  // P
    {0x3E, 0x41, 0x61, 0x7E, 0x00},  // Q
    {0x7F, 0x09, 0x19, 0x66, 0x00},  // R
    {0x46, 0x49, 0x49, 0x31, 0x00},  // S
    {0x01, 0x7F, 0x01, 0x00, 0x00},  // T
    {0x3F, 0x40, 0x40, 0x3F, 0x00},  // U
    {0x1F, 0x20, 0x40, 0x20, 0x1F},  // V
    {0x7F, 0x20, 0x18, 0x20, 0x7F},  // W
    {0x63, 0x14, 0x08, 0x14, 0x63},  // X
    {0x07, 0x08, 0x70, 0x08, 0x07},  // Y
    {0x61, 0x51, 0x49, 0x45, 0x43},  // Z
    {0x3E, 0x51, 0x49, 0x45, 0x3E},  // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00},  // 1
    {0x42, 0x61, 0x51, 0x49, 0x46},  // 2
    {0x21, 0x41, 0x45, 0x4B, 0x31},  // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10},  // 4
    {0x27, 0x45, 0x45, 0x45, 0x39},  // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30},  // 6
    {0x01, 0x71, 0x09, 0x05, 0x03},  // 7
    {0x36, 0x49, 0x49, 0x49, 0x36},  // 8
    {0x06, 0x49, 0x49, 0x29, 0x1E},  // 9
    {0x08, 0x08, 0x08, 0x08, 0x08},  // -
    {0x00, 0x24, 0x00, 0x00, 0x00}   // :
};

typedef struct flight_info {
  char icao24[7];
  char callsign[9];
  char model[128];
} flight_info;

flight_info incoming_info; //global
bool newDataAvailable = false;

int getCharIndex(char c);
void ScrollTextLoop(const char* callsign, const char* model, uint16_t delayMs);
void OnDataRecv(uint8_t *mac, uint8_t *data, uint8_t len);


int getCharIndex(char c) {
  if (c >= 'A' && c <= 'Z') {
    return c - 'A';  // A-Z maps to 0-25
  } else if (c >= 'a' && c <= 'z') {
    return c - 'a';  // a-z maps to 0-25 (same as uppercase)
  } else if (c >= '0' && c <= '9') {
    return c - '0' + 26;  // 0-9 maps to 26-35
  } else if (c == '-') {
    return 36;  // dash maps to 36
  } else if (c == ':') {
    return 37;  // colon maps to 37
  }
  return -1;  // Invalid character
}

void ScrollTextLoop(const char* callsign, const char* model, uint16_t delayMs) {
  const uint8_t charWidth = 5;
  const uint8_t spacing = 1;

  // Combine all three texts into one continuous string
  const char* text1 = "PLANE OVERHEAD: ";
  const char* text2 = callsign;
  const char* text3 = " ";
  const char* text4 = model;

  // Calculate total length and create combined string
  int totalLen = strlen(text1) + strlen(text2) + strlen(text3) + strlen(text4);
  char* combinedText = (char*)malloc(totalLen + 1);
  strcpy(combinedText, text1);
  strcat(combinedText, text2);
  strcat(combinedText, text3);
  strcat(combinedText, text4);

  int totalWidth = totalLen * (charWidth + spacing) - spacing;
  int scrollPos = PanelWidth;

  while (scrollPos > -totalWidth) {
    strip.ClearTo(RgbColor(0, 0, 0));

    // Find positions of different sections for coloring
    int text1End = strlen(text1);
    int text2End = text1End + strlen(text2);

    for (int charIdx = 0; charIdx < totalLen; charIdx++) {
      char c = combinedText[charIdx];
      if (c == ' ') continue;

      // Determine color based on which section we're in
      RgbColor currentColor;
      if (charIdx < text1End) {
        currentColor = RgbColor(255, 60, 0);
      } else if (charIdx < text2End) {
        currentColor = RgbColor(0, 255, 255);
      } else {
        currentColor = RgbColor(100, 0, 255);
      }

      int charIndex = getCharIndex(c);
      if (charIndex < 0 || charIndex >= 38) continue;

      int charStartX = scrollPos + charIdx * (charWidth + spacing);
      if (charStartX >= PanelWidth || charStartX + charWidth < 0) continue;

      for (uint8_t col = 0; col < charWidth; col++) {
        int pixelX = charStartX + col;
        if (pixelX < 0 || pixelX >= PanelWidth) continue;

        uint8_t columnData = pgm_read_byte(&(font5x7[charIndex][col]));
        for (uint8_t row = 0; row < 7; row++) {
          if (columnData & (1 << row)) {
            uint16_t pixelIndex = topo.Map(pixelX, row);
            strip.SetPixelColor(pixelIndex, currentColor);
          }
        }
      }
    }

    strip.Show();
    delay(delayMs);
    scrollPos--;
  }

  free(combinedText);

  // Clear when done
  strip.ClearTo(RgbColor(0, 0, 0));
  strip.Show();
}

void OnDataRecv(uint8_t *mac, uint8_t *data, uint8_t len) {
  if (len != sizeof(flight_info)) {
    Serial.printf("Error: Received %d bytes (expected %d)\n", len, sizeof(flight_info));
    return;
  }
  memcpy(&incoming_info, data, sizeof(flight_info));
  newDataAvailable = true;

  Serial.println("Received flight info:");
  Serial.printf("ICAO24: %.*s\n", 7, incoming_info.icao24);
  Serial.printf("Callsign: %.*s\n", 9, incoming_info.callsign);
  Serial.printf("Model: %.*s\n", 128, incoming_info.model);
}