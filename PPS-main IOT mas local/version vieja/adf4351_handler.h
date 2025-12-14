#ifndef ADF4351_HANDLER_H
#define ADF4351_HANDLER_H

#include <Arduino.h>
#include <ArduinoJson.h>

/**
 * @brief Inicializa la comunicación SPI para el módulo ADF4351.
 * Debe ser llamada en la función setup() principal.
 */
void adf4351_setup();

/**
 * @brief Procesa un comando WebSocket para controlar el sintetizador ADF4351.
 * 
 * Espera un documento JSON con la "accion": "adf4351_command" y una "sub_accion":
 * - "set_freq": Establece una nueva frecuencia. Requiere "frecuencia_hz": <valor>.
 * - "set_power": Establece la potencia de salida. Requiere "potencia": <0-3>.
 * - "enable": Habilita la salida de RF.
 * - "disable": Deshabilita la salida de RF.
 * - "get_status": Devuelve el estado actual del módulo.
 * 
 * @param clientNum El ID del cliente WebSocket que envió la petición.
 * @param doc El documento JSON que contiene los datos del comando.
 */
void handle_adf4351_command(uint8_t clientNum, StaticJsonDocument<512>& doc);

#endif // ADF4351_HANDLER_H