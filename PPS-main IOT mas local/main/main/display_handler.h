#ifndef DISPLAY_HANDLER_H
#define DISPLAY_HANDLER_H

#include <Arduino.h>

// Definición de la estructura de estado para la pantalla
// ESTE ES EL ÚNICO LUGAR DONDE DEBE ESTAR ESTA DEFINICIÓN
struct DisplayState {
  String moduleName;
  String primaryDisplay;
  String secondaryDisplay;
  String tertiaryDisplay;
};

// Declaración "extern" para que otros archivos sepan que esta variable global existe
extern DisplayState currentDisplayState;

/**
 * @brief Inicializa el display OLED. Debe llamarse en la función setup().
 * @return true si la inicialización fue exitosa, false en caso contrario.
 */
bool display_setup();

/**
 * @brief Dibuja la pantalla principal de estado general del instrumento.
 */
void showMainScreen();

/**
 * @brief Muestra un mensaje temporal simple en toda la pantalla y en el monitor serie.
 * @param message El mensaje a mostrar.
 */
void printToAll(const String &message);

/**
 * @brief Muestra un mensaje temporal centrado, en tamaño grande, en la pantalla.
 * @param message El mensaje a mostrar.
 */
void updateOledStatus(const String &message);

#endif // DISPLAY_HANDLER_H