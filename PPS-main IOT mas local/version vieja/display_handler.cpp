#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>
#include "display_handler.h"
#include "config.h" 

// ==========================================================
// DECLARACIÓN DE OBJETOS Y VARIABLES EXTERNAS
// ==========================================================
extern Adafruit_SH1106G display;
extern int webSocketClients;
extern String ipAddressLine;

// ==========================================================
// FUNCIÓN AUXILIAR DE FORMATEO (NUEVA)
// ==========================================================

/**
 * @brief Formatea un String si contiene un número a un máximo de 3 decimales.
 * 
 * Revisa si el String contiene un espacio (asumiendo un formato "valor unidad").
 * Si lo encuentra, convierte la primera parte a float, la formatea y la une
 * de nuevo con la unidad. Si no, devuelve el String original.
 * 
 * @param inputStr El String a formatear (ej: "7.150 MHz" o "Salida: ON").
 * @return El String formateado o el original.
 */
String formatNumericString(const String &inputStr) {
  // Buscar el primer espacio para separar el número de la unidad.
  int spaceIndex = inputStr.indexOf(' ');

  // Si no hay espacio o está al principio, es texto puro (ej: "Banda: 40m").
  // Devolverlo sin cambios para evitar errores.
  if (spaceIndex <= 0) {
    return inputStr;
  }

  // Separar la parte numérica y la parte de la unidad.
  String numberPart = inputStr.substring(0, spaceIndex);
  String unitPart = inputStr.substring(spaceIndex); // Esto incluye el espacio inicial.

  // Convertir la parte numérica a float. toFloat() devuelve 0.0 si falla.
  float value = numberPart.toFloat();
  
  // Reconstruir el String con el número formateado a 3 decimales y su unidad.
  return String(value, 3) + unitPart;
}

// ==========================================================
// IMPLEMENTACIÓN DE LAS FUNCIONES DE PANTALLA
// ==========================================================

bool display_setup() {
  if (!display.begin(OLED_ADDR, true)) {
    Serial.println(F("Fallo al iniciar el display SH1106G"));
    return false;
  }
  display.clearDisplay();
  printToAll("Iniciando...");
  return true;
}

void showMainScreen() {
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);

  // 1. Título dinámico
  display.setTextSize(1);
  String title = currentDisplayState.moduleName; 
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(title, 0, 0, &x1, &y1, &w, &h);
  int cursorX = (ANCHO - w) / 2;
  display.setCursor(cursorX > 0 ? cursorX : 0, 0);
  display.println(title);
  display.drawFastHLine(0, 10, ANCHO, SH110X_WHITE);

  // 2. Icono de WebSocket
  if (webSocketClients > 0) {
    display.drawPixel(125, 2, SH110X_WHITE);
    display.drawCircle(125, 2, 3, SH110X_WHITE);
    display.drawCircle(125, 2, 6, SH110X_WHITE);
  } else {
    display.drawPixel(125, 2, SH110X_WHITE);
    display.drawCircle(125, 2, 3, SH110X_WHITE);
    display.drawCircle(125, 2, 6, SH110X_WHITE);
    display.drawLine(120, 7, 128, 0, SH110X_WHITE);
  }
  
  // Muestra los datos desde la estructura de estado, aplicando el formateo
  display.setTextSize(2);
  display.setCursor(0, 18);
  // Se aplica la función de formateo aquí
  display.print(formatNumericString(currentDisplayState.primaryDisplay));

  display.setTextSize(1);
  display.setCursor(0, 40);
  // También se puede aplicar en los campos secundarios si es necesario
  display.print(currentDisplayState.secondaryDisplay);

  display.setCursor(67, 40);
  display.print(currentDisplayState.tertiaryDisplay);
  
  // 3. Línea de IP
  if (ipAddressLine.length() > 0) {
    display.setCursor(0, ALTO - 8);
    display.print(ipAddressLine);
  }

  display.display();
}

void printToAll(const String &message) {
  Serial.println(message);
  display.clearDisplay();
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.println(message);
  if (ipAddressLine.length() > 0) {
    display.setCursor(0, ALTO - 8);
    display.print(ipAddressLine);
  }
  display.display();
}

void updateOledStatus(const String &message) {
  Serial.println("OLED Status: " + message);
  display.clearDisplay();
  display.setTextSize(2);
  display.setTextColor(SH110X_WHITE);
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(message, 0, 0, &x1, &y1, &w, &h);
  int cursorX = (ANCHO - w) / 2;
  int cursorY = (ALTO - 9 - h) / 2;
  display.setCursor(cursorX, cursorY);
  display.println(message);
  if (ipAddressLine.length() > 0) {
    display.setTextSize(1);
    display.setCursor(0, ALTO - 8);
    display.print(ipAddressLine);
  }
  display.display();
}