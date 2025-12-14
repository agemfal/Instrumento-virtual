#ifndef VFO_HANDLER_H
#define VFO_HANDLER_H

#include <Arduino.h>
#include <ArduinoJson.h>

void vfo_setup();

void handleVfoCommand(uint8_t clientNum, JsonDocument& doc);

#endif // VFO_HANDLER_H