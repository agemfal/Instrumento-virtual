#include <ArduinoIoTCloud.h>
#include <Arduino_ConnectionHandler.h>

// ============================================================
// 1. CREDENCIALES (Tus claves originales)
// ============================================================
const char DEVICE_LOGIN_NAME[]  = "e60e8314-ccc2-4ca4-9896-63e86c5cb51d";
const char DEVICE_KEY[]         = "E6OD76X8RcTCDZuWthKlWec3M";

// ============================================================
// 2. PROTOTIPOS DE FUNCIONES (Callback Functions)
// Estas deben coincidir con las que tienes en cloud_bridge.cpp
// ============================================================
void onCloudSelectorChange();
void onCloudEnableChange();
void onCloudInputChange();    // <--- La función "Mágica"
void onCloudOscIdChange();
// Nota: cloud_display es de solo lectura, no necesita callback.

// ============================================================
// 3. VARIABLES DE LA NUBE (Solo las 5 permitidas)
// ============================================================
int cloud_selector;
bool cloud_enable;
String cloud_input;      // <--- Reemplaza a freq_input y botones
String cloud_display;    // <--- Nueva variable de retorno
int cloud_osc_id;

WiFiConnectionHandler *ArduinoIoTPreferredConnection;

void initProperties(){
  ArduinoCloud.setBoardId(DEVICE_LOGIN_NAME);
  ArduinoCloud.setSecretDeviceKey(DEVICE_KEY);

  // ============================================================
  // 4. MAPEO DE VARIABLES
  // ============================================================
  
  // 1. Selector de Generador
  ArduinoCloud.addProperty(cloud_selector, READWRITE, ON_CHANGE, onCloudSelectorChange);
  
  // 2. Enable/Disable
  ArduinoCloud.addProperty(cloud_enable, READWRITE, ON_CHANGE, onCloudEnableChange);
  
  // 3. Input Inteligente (Comandos y Frecuencia)
  ArduinoCloud.addProperty(cloud_input, READWRITE, ON_CHANGE, onCloudInputChange);
  
  // 4. Display de Estado (Solo lectura para la nube)
  ArduinoCloud.addProperty(cloud_display, READ, ON_CHANGE);
  
  // 5. Selector de Oscilador
  ArduinoCloud.addProperty(cloud_osc_id, READWRITE, ON_CHANGE, onCloudOscIdChange);
}