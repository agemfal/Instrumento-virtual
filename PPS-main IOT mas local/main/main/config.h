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


//#define AD9850_I2C_ADDR 8   // Dirección I2C del Arduino Nano
//#define AD9850_MAX_FREQ 40000000UL 
#define AD9850_CLK_FREQ 125000000.0 // Reloj del cristal del módulo


// Pines físicos (Usando tu grupo disponible: RX2, D4, D2)
// NOTA: RESET se asume conectado a GND físicamente, no se define aquí.
#define AD9850_PIN_W_CLK  16  // RX2
#define AD9850_PIN_FQ_UD  4   // D4
#define AD9850_PIN_DATA   2   // D2
#define AD9850_MAX_FREQ 40000000UL 
// NOTA: Los pines físicos W_CLK, FQ_UD, DATA, RESET ahora se conectan al Arduino Nano.
// Ya no se definen aquí para la ESP32.
// --- Configuración del ADF4351 ---
#define ADF4351_SS_PIN      5  // Pin para Slave Select (CS)
#define ADF4351_REF_CLK_HZ 8000000
#define ADF4351_MIN_FREQ 35000000ULL
#define ADF4351_MAX_FREQ 4400000000ULL

// --- Declaración de pasos predefinidos para ADF4351 ---
extern const uint32_t ADF4351_STEPS[7];
extern const uint8_t ADF4351_NUM_STEPS;

// Valores individuales de pasos (para uso en defines si es necesario)
#define ADF4351_STEP_10HZ       10
#define ADF4351_STEP_100HZ      100
#define ADF4351_STEP_1KHZ       1000
#define ADF4351_STEP_10KHZ      10000
#define ADF4351_STEP_100KHZ     100000
#define ADF4351_STEP_1MHZ       1000000
#define ADF4351_STEP_10MHZ      10000000

// --- Configuración de Switches RF ---
// PRIMER SWITCH RF
#define RF_SWITCH_1_PIN_1  25    // Control 1
#define RF_SWITCH_1_PIN_2  26    // Control 2
#define RF_SWITCH_1_PIN_3  27    // Control 3

// SEGUNDO SWITCH RF (sin conflictos con I2C, SPI ni AD9850)
#define RF_SWITCH_2_PIN_1  12    // Control 1
#define RF_SWITCH_2_PIN_2  13    // Control 2
#define RF_SWITCH_2_PIN_3  33    // Control 3

#endif // CONFIG_H