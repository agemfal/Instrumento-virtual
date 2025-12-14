#include <Wire.h>
#include <si5351.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include "vfo_handler.h"
#include "config.h"
#include "display_handler.h"

// ==========================================================
// DECLARACIÓN DE OBJETOS Y VARIABLES EXTERNAS
// ==========================================================
extern Si5351 si5351;
extern WebSocketsServer webSocket;
extern bool si5351_present;
// La declaración de 'currentDisplayState' se incluye a través de 'display_handler.h'

// ==========================================================
// DEFINICIONES Y VARIABLES DE ESTADO DEL VFO (Portadas del Sketch)
// ==========================================================
#define IF_FREQ_KHZ   455     // Frecuencia Intermedia en kHz
#define BAND_INIT     7       // Banda inicial al arrancar (1-21)
#define XT_CAL_F      0       // Factor de calibración

// Variables de estado del VFO
static unsigned long vfo_freq = 0;
static unsigned long vfo_fstep = 1000;
static long vfo_interfreq_khz = IF_FREQ_KHZ;
static byte vfo_stp = 4;
static byte vfo_band_count = BAND_INIT;
static bool vfo_is_tx = false;
static String vfo_band_name = "";

// Prototipos de funciones internas
void applyFrequency();
void setNextStep();
void setNextBand();
void updateDisplayVfoState();

// ==========================================================
// IMPLEMENTACIÓN DE FUNCIONES PÚBLICAS
// ==========================================================

void vfo_setup() {
  if (si5351.init(SI5351_CRYSTAL_LOAD_8PF, SI5351_CRYSTAL_FREQ, SI5351_CORRECTION)) {
      si5351.drive_strength(SI5351_CLK0, SI5351_DRIVE_8MA);
      si5351.output_enable(SI5351_CLK0, 1);
      si5351_present = true;
      printToAll("Si5351 iniciado OK.");
      
      setNextBand();
      vfo_band_count = BAND_INIT;
      setNextBand();
      
      applyFrequency();
      updateDisplayVfoState();
      delay(1500);

  } else {
      si5351_present = false;
      printToAll("Error Si5351.");
      delay(1500);
  }
}

void handleVfoCommand(uint8_t clientNum, StaticJsonDocument<512>& doc) {
  if (!si5351_present) {
    StaticJsonDocument<200> errorDoc;
    errorDoc["status"] = "error";
    errorDoc["mensaje"] = "Si5351 no encontrado. No se pueden procesar comandos de VFO.";
    String output;
    serializeJson(errorDoc, output);
    webSocket.sendTXT(clientNum, output);
    return;
  }
    
  // ==========================================================
  // == INICIO DE LA CORRECCIÓN ==
  // ==========================================================
  const char* sub_accion = doc["sub_accion"];

  // Esta comprobación es crucial. Solo se procesan los comandos si 'sub_accion' existe.
  // Si es nulo (como ocurre en el evento de conexión), este bloque se salta,
  // evitando el crash y permitiendo que se envíe el estado actual.
  if (sub_accion) {
    if (strcmp(sub_accion, "change_freq") == 0) {
      String direccion = doc["direccion"];
      if (direccion == "up") {
        vfo_freq += vfo_fstep;
        if (vfo_freq >= 225000000) vfo_freq = 225000000;
      } else if (direccion == "down") {
        vfo_freq -= vfo_fstep;
        if (vfo_freq < 10000) vfo_freq = 10000;
      }
    } else if (strcmp(sub_accion, "set_step") == 0) {
      setNextStep();
    } else if (strcmp(sub_accion, "set_band") == 0) {
      setNextBand();
    } else if (strcmp(sub_accion, "set_rxtx") == 0) {
      String modo = doc["modo"];
      vfo_is_tx = (modo == "tx");
    }
  }
  // ==========================================================
  // == FIN DE LA CORRECCIÓN ==
  // ==========================================================

  // Aplicar cambios y actualizar estado
  applyFrequency();
  updateDisplayVfoState();
  showMainScreen(); // Actualizar la pantalla física

  // Preparar y enviar respuesta con el estado actual
  StaticJsonDocument<512> responseDoc;
  responseDoc["status"] = "ok";
  responseDoc["accion"] = "respuesta_vfo";
  JsonObject data = responseDoc.createNestedObject("datos");
  data["frecuencia_hz"] = vfo_freq;
  data["paso_hz"] = vfo_fstep;
  data["banda_nombre"] = vfo_band_name;
  data["modo"] = vfo_is_tx ? "TX" : "RX";
  data["if_khz"] = vfo_interfreq_khz;

  String output;
  serializeJson(responseDoc, output);
  webSocket.sendTXT(clientNum, output);
}

// ==========================================================
// IMPLEMENTACIÓN DE FUNCIONES PRIVADAS (Lógica del VFO)
// ==========================================================

void applyFrequency() {
  long current_if_khz = vfo_is_tx ? 0 : vfo_interfreq_khz;
  unsigned long long freq_if_hz = (unsigned long long)vfo_freq + (current_if_khz * 1000ULL);
  si5351.set_freq(freq_if_hz * 100ULL, SI5351_CLK0);
}

void setNextStep() {
  vfo_stp++;
  if (vfo_stp > 6) vfo_stp = 1;
  
  switch (vfo_stp) {
    case 1: vfo_fstep = 1; break;
    case 2: vfo_fstep = 10; break;
    case 3: vfo_fstep = 1000; break;
    case 4: vfo_fstep = 5000; break;
    case 5: vfo_fstep = 10000; break;
    case 6: vfo_fstep = 1000000; break;
  }
}

void setNextBand() {
  vfo_band_count++;
  if (vfo_band_count > 21) vfo_band_count = 1;

  switch (vfo_band_count) {
    case 1: vfo_freq = 100000; vfo_band_name = "GEN"; break;
    case 2: vfo_freq = 800000; vfo_band_name = "MW"; break;
    case 3: vfo_freq = 1800000; vfo_band_name = "160m"; break;
    case 4: vfo_freq = 3650000; vfo_band_name = "80m"; break;
    case 5: vfo_freq = 4985000; vfo_band_name = "60m"; break;
    case 6: vfo_freq = 6180000; vfo_band_name = "49m"; break;
    case 7: vfo_freq = 7200000; vfo_band_name = "40m"; break;
    case 8: vfo_freq = 10000000; vfo_band_name = "31m"; break;
    case 9: vfo_freq = 11780000; vfo_band_name = "25m"; break;
    case 10: vfo_freq = 13630000; vfo_band_name = "22m"; break;
    case 11: vfo_freq = 14100000; vfo_band_name = "20m"; break;
    case 12: vfo_freq = 15000000; vfo_band_name = "19m"; break;
    case 13: vfo_freq = 17655000; vfo_band_name = "16m"; break;
    case 14: vfo_freq = 21525000; vfo_band_name = "13m"; break;
    case 15: vfo_freq = 27015000; vfo_band_name = "11m"; break;
    case 16: vfo_freq = 28400000; vfo_band_name = "10m"; break;
    case 17: vfo_freq = 50000000; vfo_band_name = "6m"; break;
    case 18: vfo_freq = 100000000; vfo_band_name = "WFM"; break;
    case 19: vfo_freq = 130000000; vfo_band_name = "AIR"; break;
    case 20: vfo_freq = 144000000; vfo_band_name = "2m"; break;
    case 21: vfo_freq = 220000000; vfo_band_name = "1m"; break;
  }
  si5351.pll_reset(SI5351_PLLA);
}

void updateDisplayVfoState() {
    currentDisplayState.moduleName = "Si5351  (" + String(vfo_is_tx ? "TX" : "RX") + ")";
    
    double freqMHz = vfo_freq / 1000000.0;
    currentDisplayState.primaryDisplay = String(freqMHz, 3) + " MHz";

    String stepStr;
    if(vfo_fstep < 1000) stepStr = String(vfo_fstep) + "Hz";
    else stepStr = String(vfo_fstep / 1000) + "kHz";
    currentDisplayState.secondaryDisplay = "Banda: " + vfo_band_name;
    
    currentDisplayState.tertiaryDisplay = "Paso: " + stepStr;
}