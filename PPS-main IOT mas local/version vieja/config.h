#ifndef CONFIG_H
#define CONFIG_H

// ==========================================================
// SECCIÓN DE DEFINICIONES Y CONSTANTES GLOBALES
// ==========================================================

// --- Configuración del Display OLED ---
#define ANCHO 128
#define ALTO 64
#define OLED_ADDR 0x3C

// --- Configuración del Si5351 ---
#define SI5351_CRYSTAL_FREQ 25000000UL
#define SI5351_CORRECTION   0
#define SI5351_ADDR         0x60

// --- Configuración del AD9850 ---
#define AD9850_W_CLK_PIN 15  // Pin para Word Clock (CLK)
#define AD9850_FQ_UD_PIN 2   // Pin para Frequency Update (FQ)
#define AD9850_DATA_PIN  4   // Pin para Data (DATA)
#define AD9850_RESET_PIN 16  // Pin para Reset (RESET)
#define AD9850_MAX_FREQ 40000000UL // Límite de frecuencia seguro para el AD9850

#define ADF4351_SS_PIN 5
#define ADF4351_REF_CLK_HZ 25000000
#define ADF4351_MIN_FREQ 35000000ULL
#define ADF4351_MAX_FREQ 4400000000ULL

// --- Configuración de Switches RF ---
// PRIMER SWITCH RF (sin conflictos)
#define RF_SWITCH_1_PIN_1  25    // Control 1
#define RF_SWITCH_1_PIN_2  26    // Control 2
#define RF_SWITCH_1_PIN_3  27    // Control 3

// SEGUNDO SWITCH RF (movido a pines seguros y sin conflictos)
#define RF_SWITCH_2_PIN_1  14    // Control 1
#define RF_SWITCH_2_PIN_2  32    // Control 2
#define RF_SWITCH_2_PIN_3  33    // Control 3


#endif // CONFIG_H
