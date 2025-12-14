#ifndef AD9850_HANDLER_H
#define AD9850_HANDLER_H

#include <Arduino.h>
#include <ArduinoJson.h>

/**
 * @brief Inicializa los pines de control para el módulo AD9850.
 * Debe ser llamada en la función setup() principal.
 */
void ad9850_setup();

/**
 * @brief Procesa un comando WebSocket para controlar el VFO AD9850.
 * 
 * Espera un documento JSON con la "accion": "ad9850_command" y una "sub_accion":
 * - "set_freq": Establece una nueva frecuencia. Requiere "frecuencia_hz": <valor>.
 * - "enable": Habilita la salida de RF (establece la última frecuencia configurada).
 * - "disable": Deshabilita la salida de RF (establece la frecuencia en 0 Hz).
 * - "get_status": Devuelve el estado actual del módulo.
 * 
 * @param clientNum El ID del cliente WebSocket que envió la petición.
 * @param doc El documento JSON que contiene los datos del comando.
 */
void handle_ad9850_command(uint8_t clientNum, StaticJsonDocument<512>& doc);

#endif // AD9850_HANDLER_H