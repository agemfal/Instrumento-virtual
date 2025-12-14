#include <WebSocketsServer.h>
#include "ad9850_handler.h"
#include "display_handler.h"
#include "config.h"
// ==========================================================
// DEFINICIONES Y PINES (Adaptado de AD9850.txt)
// ==========================================================


// Frecuencia del oscilador de cristal del módulo AD9850
const double ad9850_clock_freq = 125000000.0;

// ==========================================================
// VARIABLES DE ESTADO
// ==========================================================
extern WebSocketsServer webSocket;
extern DisplayState currentDisplayState; // <-- IMPORTANTE: Acceso a la estructura global
static bool ad9850_is_enabled = false;
static uint32_t ad9850_current_freq_hz = 1000000; // Frecuencia por defecto: 1 MHz

// ==========================================================
// FUNCIONES PRIVADAS DE CONTROL
// ==========================================================

// Pulso rápido en un pin (HIGH y luego LOW)
void pulse_high(int pin) {
  digitalWrite(pin, HIGH);
  digitalWrite(pin, LOW);
}

// Transfiere un byte de datos al AD9850 de forma serial (bit-banging)
void tfr_byte(byte data) {
  for (int i = 0; i < 8; i++, data >>= 1) {
    digitalWrite(AD9850_DATA_PIN, data & 0x01);
    pulse_high(AD9850_W_CLK_PIN);
  }
}

// Envía la palabra de configuración de frecuencia completa al AD9850
void send_frequency(uint32_t frequency) {
  // Fórmula de cálculo: tuning_word = frequency * 2^32 / clock_freq
  uint32_t tuning_word = (uint32_t)((double)frequency * 4294967295.0 / ad9850_clock_freq);

  for (int b = 0; b < 4; b++, tuning_word >>= 8) {
    tfr_byte(tuning_word & 0xFF);
  }
  tfr_byte(0x00); // Byte de control final (fase 0, power-down off)
  pulse_high(AD9850_FQ_UD_PIN);
}

/**
 * @brief Actualiza la estructura global de estado del display con los datos del AD9850.
 *        Esta función es el equivalente a 'updateDisplayVfoState' para este módulo.
 */
void updateDisplayAd9850State() {
    currentDisplayState.moduleName = "AD9850 (" + String(ad9850_is_enabled ? "ON" : "OFF") + ")";
    
    String freq_str;
    if (ad9850_current_freq_hz >= 1000000) {      // Si es 1 MHz o más
        double freqMHz = ad9850_current_freq_hz / 1000000.0;
        freq_str = String(freqMHz, 3) + " MHz";
    } else if (ad9850_current_freq_hz >= 1000) { // Si es 1 kHz o más
        double freqKHz = ad9850_current_freq_hz / 1000.0;
        freq_str = String(freqKHz, 3) + " kHz";
    } else {                                     // Si es menor a 1 kHz
        freq_str = String(ad9850_current_freq_hz) + " Hz";
    }
    currentDisplayState.primaryDisplay = freq_str; // <-- Ahora usa la unidad correcta

    currentDisplayState.secondaryDisplay = "Salida:" + String(ad9850_is_enabled ? " On" : "Off");
    currentDisplayState.tertiaryDisplay = "";
}
// ==========================================================
// IMPLEMENTACIÓN DE FUNCIONES PÚBLICAS
// ==========================================================

void ad9850_setup() {
  pinMode(AD9850_W_CLK_PIN, OUTPUT);
  pinMode(AD9850_FQ_UD_PIN, OUTPUT);
  pinMode(AD9850_DATA_PIN, OUTPUT);
  pinMode(AD9850_RESET_PIN, OUTPUT);

  pulse_high(AD9850_RESET_PIN);
  pulse_high(AD9850_W_CLK_PIN);
  pulse_high(AD9850_FQ_UD_PIN); // Habilita el modo de carga serial

  // Por defecto, se inicia deshabilitado (frecuencia 0)
  send_frequency(0); 
  Serial.println("Modulo AD9850 inicializado.");
}

void handle_ad9850_command(uint8_t clientNum, StaticJsonDocument<512>& doc) {
  const char* sub_accion = doc["sub_accion"];

  if (sub_accion) {
    if (strcmp(sub_accion, "set_freq") == 0) {
      if (doc.containsKey("frecuencia_hz")) {
        uint32_t new_freq = doc["frecuencia_hz"];
        if (new_freq <= AD9850_MAX_FREQ) {
          ad9850_current_freq_hz = new_freq;
          if (ad9850_is_enabled) {
            send_frequency(ad9850_current_freq_hz);
          }
        } else {
          // Frecuencia fuera de rango, no se hace nada o se envía error
        }
      }
    } else if (strcmp(sub_accion, "enable") == 0) {
      ad9850_is_enabled = true;
      send_frequency(ad9850_current_freq_hz);
    } else if (strcmp(sub_accion, "disable") == 0) {
      ad9850_is_enabled = false;
      send_frequency(0); // Poner la frecuencia a 0 deshabilita la salida
    }
  }

  // Actualizar la estructura del display ANTES de redibujar la pantalla
  updateDisplayAd9850State();
  
  // Enviar respuesta con el estado actual
  StaticJsonDocument<256> responseDoc;
  responseDoc["status"] = "ok";
  responseDoc["accion"] = "respuesta_ad9850";
  JsonObject data = responseDoc.createNestedObject("datos");
  data["frecuencia_hz"] = ad9850_current_freq_hz;
  data["habilitado"] = ad9850_is_enabled;

  String output;
  serializeJson(responseDoc, output);
  webSocket.sendTXT(clientNum, output);
  
  // La llamada a showMainScreen ya no necesita delay, es más responsivo
  showMainScreen();
}