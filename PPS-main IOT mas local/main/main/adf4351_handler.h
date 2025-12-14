#ifndef ADF4351_HANDLER_H
#define ADF4351_HANDLER_H

#include <Arduino.h>
#include <ArduinoJson.h>

void adf4351_setup();

void handle_adf4351_command(uint8_t clientNum, JsonDocument& doc);

#endif // ADF4351_HANDLER_H