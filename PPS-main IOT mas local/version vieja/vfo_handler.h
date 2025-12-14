#ifndef VFO_HANDLER_H
#define VFO_HANDLER_H

#include <Arduino.h>
#include <ArduinoJson.h>

/**
 * @brief Inicializa el Si5351 con la configuración del VFO y los valores por defecto.
 * Debe ser llamada en la función setup() principal.
 */
void vfo_setup();

/**
 * @brief Procesa un comando WebSocket para controlar el VFO.
 * 
 * Espera un documento JSON con un campo "sub_accion", que puede ser:
 * - "change_freq": Modifica la frecuencia. Requiere "direccion":"up" o "down".
 * - "set_step": Cambia al siguiente paso de frecuencia.
 * - "set_band": Cambia a la siguiente banda predefinida.
 * - "set_rxtx": Cambia entre RX y TX. Requiere "modo":"rx" o "tx".
 * - "get_status": Devuelve el estado actual del VFO.
 * 
 * @param clientNum El ID del cliente WebSocket que envió la petición.
 * @param doc El documento JSON que contiene los datos del comando.
 */
void handleVfoCommand(uint8_t clientNum, StaticJsonDocument<512>& doc);

#endif // VFO_HANDLER_H