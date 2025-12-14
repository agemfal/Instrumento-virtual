#include <SPI.h>
#include <WebSocketsServer.h>
#include "adf4351_handler.h"
#include "display_handler.h"
#include "config.h"

// ==========================================================
// VARIABLES DE ESTADO Y REGISTROS
// ==========================================================
extern WebSocketsServer webSocket;
extern DisplayState currentDisplayState;

struct Adf4351State {
  unsigned long long frequency_hz;
  bool rf_enabled;
  uint8_t out_power; // 0 (-4dBm), 1 (-1dBm), 2 (+2dBm), 3 (+5dBm)
  uint32_t registers[6];
};

static Adf4351State adf_state = {1000000000ULL, false, 3, {0,0,0,0,0,0}};

// Macro para desplazamiento de bits (corregida)
#define SHL(x, y) ((uint32_t)(x) << (y))

// ==========================================================
// FUNCIONES PRIVADAS DE CONTROL
// ==========================================================

/**
 * @brief Actualiza la estructura global de estado del display con los datos del ADF4351.
 */
void updateDisplayAdf4351State() {
    currentDisplayState.moduleName = "ADF4351 (" + String(adf_state.rf_enabled ? "ON" : "OFF") + ")";

    String freq_str;
    if (adf_state.frequency_hz >= 1000000000ULL) {
        freq_str = String(adf_state.frequency_hz / 1000000000.0, 4) + " GHz";
    } else {
        freq_str = String(adf_state.frequency_hz / 1000000.0, 4) + " MHz";
    }
    currentDisplayState.primaryDisplay = freq_str;

    const char* powerLevels[] = {"-4dBm", "-1dBm", "+2dBm", "+5dBm"};
    currentDisplayState.secondaryDisplay = "Pot: " + String(powerLevels[adf_state.out_power]);
    currentDisplayState.tertiaryDisplay = "Salida: " + String(adf_state.rf_enabled ? "ON" : "OFF");
}

// Calcula y actualiza los 6 registros de 32 bits del ADF4351
void prepare_registers() {
    uint8_t rf_div_sel = 0;
    if(adf_state.frequency_hz >= 2200000000ULL) rf_div_sel = 0;
    else if(adf_state.frequency_hz >= 1100000000ULL) rf_div_sel = 1;
    else if(adf_state.frequency_hz >= 550000000ULL) rf_div_sel = 2;
    else if(adf_state.frequency_hz >= 275000000ULL) rf_div_sel = 3;
    else if(adf_state.frequency_hz >= 137500000ULL) rf_div_sel = 4;
    else if(adf_state.frequency_hz >= 68750000ULL) rf_div_sel = 5;
    else rf_div_sel = 6;

    uint32_t pfd_freq = ADF4351_REF_CLK_HZ; // Debe estar definido en config.h
    
    // Calcular frecuencia VCO
    double vco_freq = (double)adf_state.frequency_hz * (1ULL << rf_div_sel);
    
    // Calcular INT, MOD y FRAC
    uint32_t INT = (uint32_t)(vco_freq / pfd_freq);
    uint16_t MOD = 4095;
    double frac_part = (vco_freq / (double)pfd_freq) - INT;
    uint16_t FRAC = (uint16_t)(frac_part * MOD + 0.5); // Redondeo
    
    // Registro 0: Control de frecuencia
    adf_state.registers[0] = SHL(INT, 15) | SHL(FRAC, 3) | 0b000;
    
    // Registro 1: Control de fase y MOD
    adf_state.registers[1] = SHL(0, 28) | SHL(1, 27) | SHL(1, 15) | SHL(MOD, 3) | 0b001;
    
    // Registro 2: Control de charge pump y otros
    adf_state.registers[2] = SHL(0, 29) | SHL(6, 26) | SHL(0, 25) | SHL(0, 24) | 
                            SHL(1, 14) | SHL(0, 13) | SHL(7, 9) | SHL(1, 8) | 
                            SHL(0, 7) | SHL(1, 6) | SHL(0, 5) | SHL(0, 4) | 0b010;
    
    // Registro 3: Control de temporización
    adf_state.registers[3] = SHL(0, 23) | SHL(0, 22) | SHL(0, 21) | SHL(0, 18) | 
                            SHL(0, 15) | SHL(150, 3) | 0b011;
    
    // Registro 4: Control de salida RF
    adf_state.registers[4] = SHL(1, 23) | SHL(rf_div_sel, 20) | SHL(8, 12) | 
                            SHL(0, 11) | SHL(1, 10) | SHL(0, 9) | SHL(0, 8) | 
                            SHL(0, 6) | SHL(adf_state.rf_enabled, 5) | 
                            SHL(adf_state.out_power, 3) | 0b100;
    
    // Registro 5: Control de pin LD
    adf_state.registers[5] = SHL(1, 22) | SHL(0b11, 19) | 0b101;
}

// Envía todos los registros al chip
void update_all_registers() {
    digitalWrite(ADF4351_SS_PIN, LOW);
    delayMicroseconds(10);
    
    for (int i = 5; i >= 0; i--) {
        uint32_t reg_val = adf_state.registers[i];
        
        // Transferir en orden MSB first
        SPI.transfer((reg_val >> 24) & 0xFF);
        SPI.transfer((reg_val >> 16) & 0xFF);
        SPI.transfer((reg_val >> 8) & 0xFF);
        SPI.transfer(reg_val & 0xFF);
        
        digitalWrite(ADF4351_SS_PIN, HIGH);
        delayMicroseconds(10);
        digitalWrite(ADF4351_SS_PIN, LOW);
        delayMicroseconds(10);
    }
    
    digitalWrite(ADF4351_SS_PIN, HIGH);
    delayMicroseconds(100);
}

// ==========================================================
// IMPLEMENTACIÓN DE FUNCIONES PÚBLICAS
// ==========================================================

void adf4351_setup() {
    pinMode(ADF4351_SS_PIN, OUTPUT);
    digitalWrite(ADF4351_SS_PIN, HIGH);
    
    SPI.begin();
    SPI.setDataMode(SPI_MODE0);
    SPI.setBitOrder(MSBFIRST);
    
    prepare_registers();
    update_all_registers();
    
    Serial.println("Modulo ADF4351 inicializado.");
}

void handle_adf4351_command(uint8_t clientNum, StaticJsonDocument<512>& doc) {
    const char* sub_accion = doc["sub_accion"];
    bool needs_update = false;

    if (sub_accion) {
        if (strcmp(sub_accion, "set_freq") == 0) {
            unsigned long long new_freq = doc["frecuencia_hz"];
            if (new_freq >= ADF4351_MIN_FREQ && new_freq <= ADF4351_MAX_FREQ) {
                adf_state.frequency_hz = new_freq;
                needs_update = true;
                Serial.println("Frecuencia ADF4351 actualizada: " + String(new_freq));
            }
        } else if (strcmp(sub_accion, "set_power") == 0) {
            uint8_t new_power = doc["potencia"];
            if (new_power <= 3) {
                adf_state.out_power = new_power;
                needs_update = true;
                Serial.println("Potencia ADF4351 actualizada: " + String(new_power));
            }
        } else if (strcmp(sub_accion, "enable") == 0) {
            adf_state.rf_enabled = true;
            needs_update = true;
            Serial.println("Salida RF ADF4351 habilitada");
        } else if (strcmp(sub_accion, "disable") == 0) {
            adf_state.rf_enabled = false;
            needs_update = true;
            Serial.println("Salida RF ADF4351 deshabilitada");
        } else if (strcmp(sub_accion, "get_status") == 0) {
            // Solo enviar estado, sin actualizar hardware
        }

        if (needs_update) {
            prepare_registers();
            update_all_registers();
        }
    }

    // Actualizar la estructura del display
    updateDisplayAdf4351State();
    
    // Enviar respuesta con el estado actual
    StaticJsonDocument<512> responseDoc;
    responseDoc["status"] = "ok";
    responseDoc["accion"] = "respuesta_adf4351";
    JsonObject data = responseDoc.createNestedObject("datos");
    data["frecuencia_hz"] = String(adf_state.frequency_hz); // Como string para evitar problemas de precisión
    data["potencia"] = adf_state.out_power;
    data["habilitado"] = adf_state.rf_enabled;

    String output;
    serializeJson(responseDoc, output);
    webSocket.sendTXT(clientNum, output);
    
    // Actualizar pantalla
    showMainScreen();
}