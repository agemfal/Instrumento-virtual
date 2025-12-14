#include <WiFi.h>
#include <WebServer.h>
#include <EEPROM.h>
#include "portal_config.h"

// ==========================================================
// DECLARACIÓN DE OBJETOS Y FUNCIONES EXTERNAS
// ==========================================================
// Estos objetos y variables están definidos en el archivo principal.
// Se declaran aquí como 'extern' para que este archivo sepa de su existencia.

extern WebServer server;
extern const char* ap_ssid;
extern const char* ap_pass;

/**
 * @brief Muestra un mensaje en el display OLED y en el monitor serie.
 * 
 * @param message El mensaje a mostrar.
 */
extern void printToAll(const String &message);


// ==========================================================
// IMPLEMENTACIÓN DE LA LÓGICA DEL PORTAL
// ==========================================================

/**
 * @brief Maneja las peticiones a la raíz ("/") del servidor web,
 * mostrando el formulario de configuración.
 */
// Guardamos la página HTML directamente en la memoria Flash
const char page_root[] PROGMEM =
"<!DOCTYPE html><html><head><title>Configurar Wi-Fi</title>"
"<meta name='viewport' content='width=device-width,initial-scale=1'>"
"<style>"
"body{font-family:sans-serif;text-align:center;padding:20px}"
"h1{color:#333}"
"form{display:inline-block;background:#f2f2f2;padding:20px;border-radius:8px}"
"input{width:100%;padding:10px;margin:6px 0;border:1px solid #ccc;box-sizing:border-box}"
"input[type=submit]{background:#4CAF50;color:#fff;border:none;cursor:pointer}"
"input[type=submit]:hover{background:#45a049}"
"</style></head>"
"<body><h1>Conectar ESP32 a Wi-Fi</h1>"
"<form action='/save' method='POST'>"
"SSID:<input type='text' name='ssid'><br>"
"Contraseña:<input type='password' name='pass'><br>"
"<input type='submit' value='Guardar y Reiniciar'>"
"</form></body></html>";

// Esta función ahora solo envía el HTML desde PROGMEM (sin ocupar RAM)
void handleRoot() {
  server.send_P(200, "text/html", page_root);
}


/**
 * @brief Maneja el envío del formulario desde "/save". Guarda las
 * credenciales en la EEPROM y reinicia el dispositivo.
 */
void handleSave() {
  if (server.hasArg("ssid") && server.hasArg("pass")) {
    String ssidInput = server.arg("ssid");
    String passInput = server.arg("pass");
    EEPROM.begin(512);
    EEPROM.writeString(0, ssidInput);
    EEPROM.writeString(100, passInput);
    EEPROM.commit();
    EEPROM.end();
    printToAll("Datos guardados!\nReiniciando...");
    server.send(200, "text/html", "<h1>Datos guardados! Reiniciando ESP32...</h1>");
    delay(2000);
    ESP.restart();
  } else {
    server.send(400, "text/plain", "Faltan datos");
  }
}

/**
 * @brief Inicia el WiFi en modo AP, configura las rutas del servidor web
 * y lo pone en marcha.
 */
void startAP() {
  WiFi.softAP(ap_ssid, ap_pass);
  String msg = "Modo AP activado\n";
  msg += "Red: " + String(ap_ssid) + "\n";
  msg += "IP: " + WiFi.softAPIP().toString();
  printToAll(msg);
  server.on("/", handleRoot);
  server.on("/save", HTTP_POST, handleSave);
  server.begin();
}