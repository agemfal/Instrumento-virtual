#ifndef AD9850_HANDLER_H
#define AD9850_HANDLER_H

#include <Arduino.h>
#include <ArduinoJson.h>

void ad9850_setup();

void handle_ad9850_command(uint8_t clientNum, JsonDocument& doc);

#endif // AD9850_HANDLER_H