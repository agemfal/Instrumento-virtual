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

// --- NUEVO: Traemos la variable de la Nube ---
extern String cloud_display; 

// ==========================================================
// FUNCIÓN AUXILIAR DE FORMATEO
// ==========================================================
String formatNumericString(const String &inputStr) {
  int spaceIndex = inputStr.indexOf(' ');
  if (spaceIndex <= 0) return inputStr;

  String numberPart = inputStr.substring(0, spaceIndex);
  String unitPart = inputStr.substring(spaceIndex); 
  float value = numberPart.toFloat();
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
  printToAll("Iniciando Sistema...");
  return true;
}

void showMainScreen() {
  // 1. ACTUALIZAR HARDWARE (OLED)
  display.clearDisplay();
  display.setTextColor(SH110X_WHITE);

  // Título
  display.setTextSize(1);
  String title = currentDisplayState.moduleName; 
  int16_t x1, y1;
  uint16_t w, h;
  display.getTextBounds(title, 0, 0, &x1, &y1, &w, &h);
  int cursorX = (ANCHO - w) / 2;
  display.setCursor(cursorX > 0 ? cursorX : 0, 0);
  display.println(title);
  display.drawFastHLine(0, 10, ANCHO, SH110X_WHITE);

  // Icono WebSocket
  if (webSocketClients > 0) {
    display.fillCircle(125, 2, 2, SH110X_WHITE); // Círculo lleno si conectado
  } else {
    display.drawCircle(125, 2, 2, SH110X_WHITE); // Círculo vacío si no
    display.drawLine(120, 7, 128, 0, SH110X_WHITE);
  }
  
  // Datos principales formateados
  String formattedPrimary = formatNumericString(currentDisplayState.primaryDisplay);

  display.setTextSize(2);
  display.setCursor(0, 18);
  display.print(formattedPrimary);

  display.setTextSize(1);
  display.setCursor(0, 40);
  display.print(currentDisplayState.secondaryDisplay);

  display.setCursor(67, 40);
  display.print(currentDisplayState.tertiaryDisplay);
  
  // IP
  if (ipAddressLine.length() > 0) {
    display.setCursor(0, ALTO - 8);
    display.print(ipAddressLine);
  }

  display.display();

  // 2. ACTUALIZAR NUBE (Sincronización)
  // Creamos un string resumen: "VFO: 7.100 MHz | RX Step:1k"
  String cloudMsg = title + ": " + formattedPrimary;
  
  // Añadimos información secundaria si existe
  if (currentDisplayState.secondaryDisplay.length() > 0) {
    cloudMsg += " | " + currentDisplayState.secondaryDisplay;
  }
  if (currentDisplayState.tertiaryDisplay.length() > 0) {
    cloudMsg += " " + currentDisplayState.tertiaryDisplay;
  }

  // Solo actualizamos la variable si ha cambiado para no saturar el tráfico
  if (cloud_display != cloudMsg) {
    cloud_display = cloudMsg;
    // No hacemos Serial.println aquí para no spamear el puerto serie
  }
}

void printToAll(const String &message) {
  Serial.println(message);
  
  // Hardware OLED
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

  // Nube
  cloud_display = "[INFO] " + message;
}

void updateOledStatus(const String &message) {
  Serial.println("OLED Status: " + message);
  
  // Hardware OLED
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

  // Nube
  // Enviamos el mensaje directo a la nube
  cloud_display = message;
}