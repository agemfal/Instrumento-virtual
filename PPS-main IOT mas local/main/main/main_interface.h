#ifndef MAIN_INTERFACE_H
#define MAIN_INTERFACE_H

#include <ArduinoJson.h>

#define CLOUD_CLIENT_ID 255 

// Debe decir JsonDocument&
void ejecutarComandoCentral(uint8_t clientNum, JsonDocument& doc);

#endif