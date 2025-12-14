#include "rf_switch_handler.h"
#include "config.h"

static void set_pins_binary(uint8_t pin1, uint8_t pin2, uint8_t pin3, uint8_t value) {
    digitalWrite(pin1, (value >> 0) & 1);
    digitalWrite(pin2, (value >> 1) & 1);
    digitalWrite(pin3, (value >> 2) & 1);
}

void rf_switch_setup() {
    pinMode(RF_SWITCH_1_PIN_1, OUTPUT);
    pinMode(RF_SWITCH_1_PIN_2, OUTPUT);
    pinMode(RF_SWITCH_1_PIN_3, OUTPUT);
    pinMode(RF_SWITCH_2_PIN_1, OUTPUT);
    pinMode(RF_SWITCH_2_PIN_2, OUTPUT);
    pinMode(RF_SWITCH_2_PIN_3, OUTPUT);
    select_generator(0);
    select_oscillator(0);
    Serial.println("RF Switch Handler (simplificado) inicializado.");
}

void select_generator(uint8_t generator_id) {
    if (generator_id > 7) return;
    Serial.printf("[Switch] Seleccionando Generador: %d\n", generator_id);
    set_pins_binary(RF_SWITCH_1_PIN_1, RF_SWITCH_1_PIN_2, RF_SWITCH_1_PIN_3, generator_id);
}

void select_oscillator(uint8_t oscillator_id) {
    if (oscillator_id > 7) return;
    Serial.printf("[Switch] Seleccionando Oscilador: %d\n", oscillator_id);
    set_pins_binary(RF_SWITCH_2_PIN_1, RF_SWITCH_2_PIN_2, RF_SWITCH_2_PIN_3, oscillator_id);
}