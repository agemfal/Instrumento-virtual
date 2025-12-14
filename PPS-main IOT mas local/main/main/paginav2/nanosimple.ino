#include <Wire.h>
#include <SPI.h>
//  ES EL  ,AS SIMPLE CUENTA PULSOS
// =====================
// PINES DE MONITOREO
// =====================
#define SI5351_ADDR 0x60
#define AD9850_W_CLK_PIN 2
#define AD9850_FQ_UD_PIN 3
#define ADF4351_SS_PIN 10

// =====================
// CONTADORES DE ACTIVIDAD
// =====================
volatile unsigned long i2c_activity_count = 0;
volatile unsigned long ad9850_clk_pulse_count = 0;
volatile unsigned long ad9850_fqud_pulse_count = 0;
volatile unsigned long spi_byte_received_count = 0;

void setup() {
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.println("\n[SIMULADOR DE TRÁFICO] Iniciado. Monitoreando actividad...");

  // --- I2C ---
  Wire.begin(SI5351_ADDR);
  Wire.onReceive(i2cEvent);
  Serial.println("[INFO] Escuchando en bus I2C...");

  // --- AD9850 ---
  pinMode(AD9850_W_CLK_PIN, INPUT);
  pinMode(AD9850_FQ_UD_PIN, INPUT);
  // Usar interrupciones externas, son las más fiables para detectar pulsos
  attachInterrupt(digitalPinToInterrupt(AD9850_W_CLK_PIN), ad9850_clk_isr, RISING);
  attachInterrupt(digitalPinToInterrupt(AD9850_FQ_UD_PIN), ad9850_fqud_isr, RISING);
  Serial.println("[INFO] Escuchando pulsos en D2 (W_CLK) y D3 (FQ_UD)...");

  // --- ADF4351 ---
  pinMode(MISO, OUTPUT);
  pinMode(ADF4351_SS_PIN, INPUT_PULLUP);
  SPCR |= _BV(SPE);
  SPCR |= _BV(SPIE);
  Serial.println("[INFO] Escuchando en bus SPI...");
}

void loop() {
  // 1. "Latido del corazón" para saber que el Nano no está colgado
  static unsigned long lastBlink = 0;
  if (millis() - lastBlink > 500) {
    digitalWrite(LED_BUILTIN, !digitalRead(LED_BUILTIN));
    lastBlink = millis();
  }
  
  // 2. Reportar el tráfico detectado por las interrupciones
  reportActivity();
}

void reportActivity() {
  // Deshabilitar interrupciones para leer contadores de forma segura
  noInterrupts();
  unsigned long i2c_count = i2c_activity_count;
  unsigned long clk_count = ad9850_clk_pulse_count;
  unsigned long fqud_count = ad9850_fqud_pulse_count;
  unsigned long spi_count = spi_byte_received_count;
  
  // Resetear contadores
  i2c_activity_count = 0;
  ad9850_clk_pulse_count = 0;
  ad9850_fqud_pulse_count = 0;
  spi_byte_received_count = 0;
  interrupts(); // Volver a habilitar

  // Imprimir solo si hubo actividad
  if (i2c_count > 0) {
    Serial.print("[TRÁFICO I2C] Detectada transmisión I2C de ");
    Serial.print(i2c_count);
    Serial.println(" bytes.");
  }
  if (clk_count > 0) {
    Serial.print("[TRÁFICO AD9850] Detectados ");
    Serial.print(clk_count);
    Serial.println(" pulsos en W_CLK (D2).");
  }
  if (fqud_count > 0) {
    Serial.println("[TRÁFICO AD9850] ¡Detectado 1 pulso en FQ_UD (D3)!");
  }
  if (spi_count > 0) {
    Serial.print("[TRÁFICO SPI] Detectados ");
    Serial.print(spi_count);
    Serial.println(" bytes recibidos.");
  }
}

// =====================
// RUTINAS DE INTERRUPCIÓN (ISRs)
// =====================

// --- I2C ---
void i2cEvent(int howMany) {
  i2c_activity_count = howMany;
  while(Wire.available()) Wire.read(); // Limpiar el buffer
}

// --- AD9850 ---
void ad9850_clk_isr() {
  ad9850_clk_pulse_count++;
}

void ad9850_fqud_isr() {
  ad9850_fqud_pulse_count++;
}

// --- ADF4351 ---
ISR(SPI_STC_vect) {
  SPDR; // Leer el registro para limpiar la bandera de interrupción
  spi_byte_received_count++;
}
