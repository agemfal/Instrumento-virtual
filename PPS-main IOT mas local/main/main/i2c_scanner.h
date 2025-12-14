#ifndef I2C_SCANNER_H
#define I2C_SCANNER_H

#include <Arduino.h>

/**
 * @brief Realiza un escaneo de dispositivos en el bus I2C, actualiza la pantalla
 * y envía el resultado a un cliente WebSocket específico.
 * 
 * @param clientNum El ID del cliente WebSocket que recibirá la respuesta.
 */
void performI2CScanAndReply(uint8_t clientNum);

#endif // I2C_SCANNER_H