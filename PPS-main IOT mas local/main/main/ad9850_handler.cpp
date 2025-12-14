/* --- START OF FILE ad9850_handler.cpp --- */

#include <WebSocketsServer.h>
#include <Arduino.h> 
#include "ad9850_handler.h"
#include "display_handler.h"
#include "config.h"

// ==========================================================
// VARIABLES DE ESTADO
// ==========================================================
extern WebSocketsServer webSocket;
extern DisplayState currentDisplayState; 

static bool ad9850_is_enabled = false;
static uint32_t ad9850_current_freq_hz = 1000000; 
static uint32_t ad9850_step_hz = 1000;            

// ==========================================================
// FUNCIONES PRIVADAS (PORTADAS DEL NANO)
// ==========================================================

// Genera un pulso rápido (Copiado de tu código Nano)
void pulse_high(int pin) {
  digitalWrite(pin, HIGH); 
  delayMicroseconds(2); // Mantenemos tus 2us por seguridad con el level shifter
  digitalWrite(pin, LOW);  
  delayMicroseconds(2);
}

// Envía un byte bit a bit (Copiado de tu código Nano)
void tfr_byte(byte data) {
  for (int i = 0; i < 8; i++, data >>= 1) {
    digitalWrite(AD9850_PIN_DATA, data & 0x01);
    delayMicroseconds(1); 
    pulse_high(AD9850_PIN_W_CLK);
  }
}

// Envía la frecuencia calculada
void send_frequency(uint32_t frequency) {
  // 1. Calcular Tuning Word
  // Formula: (Freq * 2^32) / CLK_FREQ
  // Usamos double para evitar desbordamiento en la multiplicación intermedia
  uint32_t tuning_word = (uint32_t)((double)frequency * 4294967296.0 / AD9850_CLK_FREQ);

  // 2. Enviar los 4 bytes de frecuencia (LSB primero)
  for (int b = 0; b < 4; b++, tuning_word >>= 8) {
    tfr_byte(tuning_word & 0xFF);
  }

  // 3. Enviar Byte de Control (0x00 para mantener fase 0 y Power ON)
  tfr_byte(0x00); 

  // 4. Latch final para aplicar
  pulse_high(AD9850_PIN_FQ_UD);
}

void updateDisplayAd9850State() {
    currentDisplayState.moduleName = "AD9850 (" + String(ad9850_is_enabled ? "ON" : "OFF") + ")";
    
    String freq_str;
    if (ad9850_current_freq_hz >= 1000000) {      
        freq_str = String(ad9850_current_freq_hz / 1000000.0, 3) + " MHz";
    } else if (ad9850_current_freq_hz >= 1000) { 
        freq_str = String(ad9850_current_freq_hz / 1000.0, 3) + " kHz";
    } else {                                     
        freq_str = String(ad9850_current_freq_hz) + " Hz";
    }
    currentDisplayState.primaryDisplay = freq_str; 

    String step_str;
    if (ad9850_step_hz >= 1000000) step_str = "Paso: " + String(ad9850_step_hz / 1000000.0, 0) + " MHz";
    else if (ad9850_step_hz >= 1000) step_str = "Paso: " + String(ad9850_step_hz / 1000.0, 0) + " kHz";
    else step_str = "Paso: " + String(ad9850_step_hz) + " Hz";

    currentDisplayState.secondaryDisplay = step_str;
    currentDisplayState.tertiaryDisplay = ad9850_is_enabled ? "SALIDA ACTIVA" : "SALIDA APAGADA";
}

// ==========================================================
// FUNCIONES PÚBLICAS
// ==========================================================

void ad9850_setup() {
  // Configurar pines como SALIDA
  pinMode(AD9850_PIN_W_CLK, OUTPUT);
  pinMode(AD9850_PIN_FQ_UD, OUTPUT);
  pinMode(AD9850_PIN_DATA, OUTPUT);

  // Estado inicial bajo
  digitalWrite(AD9850_PIN_W_CLK, LOW);
  digitalWrite(AD9850_PIN_FQ_UD, LOW);
  digitalWrite(AD9850_PIN_DATA, LOW);

  // =========================================================
  // SECUENCIA DE INICIALIZACIÓN SERIAL (CRÍTICA)
  // =========================================================
  // Como RESET está a GND, necesitamos esta secuencia exacta
  // para habilitar el modo serial en el chip.
  pulse_high(AD9850_PIN_W_CLK);
  pulse_high(AD9850_PIN_FQ_UD);
  // =========================================================

  // Inicializar apagado (Frecuencia 0)
  send_frequency(0); 
  Serial.println("Modulo AD9850 (Directo/Serial) inicializado.");
}

void handle_ad9850_command(uint8_t clientNum, JsonDocument& doc) {
  const char* sub_accion = doc["sub_accion"];

  if (sub_accion) {
    if (strcmp(sub_accion, "set_freq") == 0) {
      if (doc.containsKey("frecuencia_hz")) {
        uint32_t new_freq = doc["frecuencia_hz"];
        if (new_freq <= AD9850_MAX_FREQ) {
          ad9850_current_freq_hz = new_freq;
          if (ad9850_is_enabled) send_frequency(ad9850_current_freq_hz);
        }
      }
    } 
    else if (strcmp(sub_accion, "change_freq") == 0) {
      const char* direccion = doc["direccion"];
      if (strcmp(direccion, "up") == 0) {
         if (ad9850_current_freq_hz + ad9850_step_hz <= AD9850_MAX_FREQ) {
             ad9850_current_freq_hz += ad9850_step_hz;
         } else {
             ad9850_current_freq_hz = AD9850_MAX_FREQ;
         }
      } else if (strcmp(direccion, "down") == 0) {
         if (ad9850_current_freq_hz >= ad9850_step_hz) {
             ad9850_current_freq_hz -= ad9850_step_hz;
         } else {
             ad9850_current_freq_hz = 0;
         }
      }
      if (ad9850_is_enabled) send_frequency(ad9850_current_freq_hz);
    }
    else if (strcmp(sub_accion, "set_step") == 0) {
      if (doc.containsKey("paso_hz")) {
        ad9850_step_hz = doc["paso_hz"];
      }
    }
    else if (strcmp(sub_accion, "enable") == 0) {
      ad9850_is_enabled = true;
      send_frequency(ad9850_current_freq_hz);
    } else if (strcmp(sub_accion, "disable") == 0) {
      ad9850_is_enabled = false;
      send_frequency(0); 
    }
  }

  updateDisplayAd9850State();
  
  StaticJsonDocument<256> responseDoc;
  responseDoc["status"] = "ok";
  responseDoc["accion"] = "respuesta_ad9850";
  
  JsonObject data = responseDoc.createNestedObject("datos");
  data["frecuencia_hz"] = ad9850_current_freq_hz;
  data["paso_hz"] = ad9850_step_hz;
  data["habilitado"] = ad9850_is_enabled;

  String output;
  serializeJson(responseDoc, output);
  if (clientNum != 255) webSocket.sendTXT(clientNum, output);
  
  showMainScreen();
}