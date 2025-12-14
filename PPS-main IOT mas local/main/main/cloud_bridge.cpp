#include <Arduino.h>
#include <ArduinoJson.h>
#include "main_interface.h"

// =====================================================
// 1. VARIABLES EXTERNAS (Coinciden con thingProperties.h)
// =====================================================
extern int cloud_selector;     // 0: VFO, 1: AD9850, 2: ADF4351
extern bool cloud_enable;      // Switch ON/OFF o RX/TX
extern String cloud_input;     // Entrada de texto
extern String cloud_display;   // Salida de texto
extern int cloud_osc_id;       // Selector auxiliar

#ifndef CLOUD_CLIENT_ID
#define CLOUD_CLIENT_ID 255
#endif

const uint8_t CLOUD_ID = CLOUD_CLIENT_ID; 

// Pasos para AD9850 y ADF4351
const uint32_t ALL_STEPS[] = {10, 100, 1000, 10000, 100000, 1000000, 10000000};
int current_step_idx = 2; 

// Potencias ADF4351
int current_pwr_idx = 3;

// =====================================================
// 2. FUNCIONES AUXILIARES
// =====================================================

void updateCloudDisplay(String msg) {
  cloud_display = msg; 
  Serial.println("[NUBE] " + msg);
}

// Helper para obtener el nombre correcto del comando JSON
String getActionName() {
  switch(cloud_selector) {
    case 0: return "vfo_command";
    case 1: return "ad9850_command";
    case 2: return "adf4351_command";
    default: return "unknown";
  }
}

// =====================================================
// 3. CALLBACKS (Funciones que llama la Nube)
// =====================================================

void onCloudSelectorChange() {
  StaticJsonDocument<256> doc;
  doc["accion"] = "oled_command";
  doc["sub_accion"] = "print";
  
  String nombre = "";
  if(cloud_selector == 0) nombre = "VFO Si5351";
  else if(cloud_selector == 1) nombre = "Gen AD9850";
  else if(cloud_selector == 2) nombre = "Synth ADF4351";
  else nombre = "Desconocido";
  
  doc["texto"] = "Nube: " + nombre;
  ejecutarComandoCentral(CLOUD_ID, doc);
  updateCloudDisplay("Activo: " + nombre);
}

// --- LÓGICA DIFERENCIADA PARA ENABLE/DISABLE ---
void onCloudEnableChange() {
  StaticJsonDocument<256> doc;
  doc["accion"] = getActionName();

  // CASO 1: VFO (Usa lógica de Radio RX/TX)
  if (cloud_selector == 0) {
    doc["sub_accion"] = "set_rxtx";
    doc["modo"] = cloud_enable ? "tx" : "rx";
    updateCloudDisplay(cloud_enable ? "VFO: TX (Transmision)" : "VFO: RX (Recepcion)");
  }
  // CASO 2: Generadores (Usa lógica de Salida ON/OFF)
  else {
    doc["sub_accion"] = cloud_enable ? "enable" : "disable";
    updateCloudDisplay(cloud_enable ? "Salida: HABILITADA" : "Salida: APAGADA");
  }
  
  ejecutarComandoCentral(CLOUD_ID, doc);
}

void onCloudOscIdChange() {
  StaticJsonDocument<256> doc;
  doc["accion"] = "select_oscillator";
  doc["id"] = cloud_osc_id;
  ejecutarComandoCentral(CLOUD_ID, doc);
  updateCloudDisplay("Oscilador HW: " + String(cloud_osc_id));
}

// --- LÓGICA DE ENTRADA (AQUÍ ESTÁ LA CORRECCIÓN PRINCIPAL) ---
void onCloudInputChange() {
  String input = cloud_input;
  input.trim();
  input.toLowerCase();

  if (input.length() == 0) return;

  StaticJsonDocument<256> doc;
  doc["accion"] = getActionName(); // Asigna vfo_command, ad9850_command, etc.

  // -------------------------------------------------
  // COMANDOS SIMPLES (+, -, s, p, e, b)
  // -------------------------------------------------
  
  if (input == "+") {
    doc["sub_accion"] = "change_freq";
    doc["direccion"] = "up";
    updateCloudDisplay("Subir Frecuencia");
  } 
  else if (input == "-") {
    doc["sub_accion"] = "change_freq";
    doc["direccion"] = "down";
    updateCloudDisplay("Bajar Frecuencia");
  }
  else if (input == "e") {
    doc["accion"] = "escanear_i2c"; // Comando especial de sistema
    ejecutarComandoCentral(CLOUD_ID, doc);
    updateCloudDisplay("Escaneando I2C...");
    cloud_input = ""; return; 
  }
  else if (input == "b" && cloud_selector == 0) { // Solo VFO tiene bandas
    doc["sub_accion"] = "set_band";
    updateCloudDisplay("Ciclar Banda VFO");
  }
  else if (input == "s") {
    // Lógica de Pasos
    current_step_idx = (current_step_idx + 1) % 7;
    doc["sub_accion"] = "set_step"; // Todos los handlers aceptan "set_step"
    
    if (cloud_selector == 0) {
      // VFO: Ignora el valor y cicla internamente. 
      // No mostramos Hz específicos en la nube para no confundir.
      updateCloudDisplay("VFO: Ciclar Paso");
    } else {
      // AD9850 / ADF4351: Necesitan el valor Hz explícito.
      doc["paso_hz"] = ALL_STEPS[current_step_idx];
      
      // Mostrar valor bonito
      String pasoStr;
      uint32_t sVal = ALL_STEPS[current_step_idx];
      if(sVal >= 1000000) pasoStr = String(sVal/1000000) + " MHz";
      else if(sVal >= 1000) pasoStr = String(sVal/1000) + " kHz";
      else pasoStr = String(sVal) + " Hz";
      updateCloudDisplay("Paso: " + pasoStr);
    }
  }
  else if (input == "p") {
    if(cloud_selector == 2) { // Solo ADF4351
      current_pwr_idx = (current_pwr_idx + 1) % 4;
      doc["sub_accion"] = "set_power";
      doc["potencia"] = current_pwr_idx;
      String dbm[] = {"-4dBm", "-1dBm", "+2dBm", "+5dBm"};
      updateCloudDisplay("Potencia: " + dbm[current_pwr_idx]);
    } else {
      updateCloudDisplay("Error: 'p' solo para ADF4351");
      return;
    }
  }
  
  // -------------------------------------------------
  // PARSEO DE FRECUENCIA DIRECTA (Números)
  // -------------------------------------------------
  else {
    // 1. Verificar si es el VFO. Si es VFO, PROHIBIR entrada directa.
    // Esto evita el fallo crítico porque vfo_handler no tiene 'set_freq'.
    if (cloud_selector == 0) {
      updateCloudDisplay("ERROR: VFO no admite Freq directa.");
      // Opcional: Podríamos enviar muchos '+' o '-', pero sería muy lento.
      cloud_input = ""; 
      return; 
    }

    // 2. Parseo para AD9850 y ADF4351
    double multiplier = 1.0;
    if (input.endsWith("g")) { multiplier = 1e9; input.remove(input.length() - 1); } 
    else if (input.endsWith("m")) { multiplier = 1e6; input.remove(input.length() - 1); } 
    else if (input.endsWith("k")) { multiplier = 1e3; input.remove(input.length() - 1); }

    double freqDouble = input.toDouble(); 
    unsigned long long freq = (unsigned long long)(freqDouble * multiplier);

    if (freq > 0) {
      // Validaciones de hardware para evitar errores lógicos en los módulos
      if (cloud_selector == 2 && freq < 35000000) {
         updateCloudDisplay("Error: Min ADF4351 es 35MHz");
         return;
      }
      
      doc["sub_accion"] = "set_freq";
      doc["frecuencia_hz"] = freq; 
      
      String unit = " Hz";
      if(freq >= 1000000000) unit = " GHz";
      else if(freq >= 1000000) unit = " MHz";
      else if(freq >= 1000) unit = " kHz";
      
      updateCloudDisplay("Set: " + String((double)freq/((freq>=1000000)?1000000.0:(freq>=1000)?1000.0:1.0)) + unit);
    } else {
      updateCloudDisplay("CMD desconocido: " + input);
      return;
    }
  }

  // Enviar el JSON construido
  ejecutarComandoCentral(CLOUD_ID, doc);
  cloud_input = ""; // Limpiar campo
}