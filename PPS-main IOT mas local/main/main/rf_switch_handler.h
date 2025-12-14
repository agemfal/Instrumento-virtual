#ifndef RF_SWITCH_HANDLER_H
#define RF_SWITCH_HANDLER_H

#include <Arduino.h>

void rf_switch_setup();
void select_generator(uint8_t generator_id);
void select_oscillator(uint8_t oscillator_id);

#endif // RF_SWITCH_HANDLER_H