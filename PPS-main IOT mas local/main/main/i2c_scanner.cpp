#include <Wire.h>
#include <ArduinoJson.h>
#include <WebSocketsServer.h>
#include "i2c_scanner.h"
#include "config.h"
#include "display_handler.h"

extern WebSocketsServer webSocket;
extern int connectedModuleCount;

void performI2CScanAndReply(uint8_t clientNum) {
  updateOledStatus("ESCANEAR I2C");

  StaticJsonDocument<512> responseDoc;
  responseDoc["status"] = "ok";
  responseDoc["accion"] = "respuesta_escaner";
  JsonArray dispositivos = responseDoc.createNestedArray("dispositivos");

  byte count = 0;
  Serial.println("Iniciando escaneo I2C...");
  for (byte address = 1; address < 127; address++) {
    if (address == OLED_ADDR) continue;

    Wire.beginTransmission(address);
    if (Wire.endTransmission() == 0) {
      Serial.printf("Dispositivo encontrado en 0x%02X\n", address);
      dispositivos.add(address);
      count++;
    }
    delay(1); // Mantenemos este delay corto para el Watchdog
  }
  
  connectedModuleCount = count;
  Serial.printf("Escaneo finalizado. Se encontraron %d dispositivos.\n", count);

  printToAll("Escaneo: " + String(count) + " disp.");
  
  // ==========================================================
  // == INICIO DE LA CORRECCIÓN ==
  // ==========================================================
  // delay(2000); // <- ELIMINA O COMENTA ESTA LÍNEA. Es la causa del problema de red.
  // ==========================================================
  // == FIN DE LA CORRECCIÓN ==
  // ==========================================================
  
  showMainScreen();

  String output;
  serializeJson(responseDoc, output);
  
  Serial.println("Enviando respuesta a cliente WebSocket:");
  Serial.println(output);
  
  webSocket.sendTXT(clientNum, output);
}