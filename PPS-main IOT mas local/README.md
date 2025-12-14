# 🌐 RF Signal Control System con ESP32

Proyecto de control remoto modular para generadores de señal de RF, basado en **ESP32** y una interfaz web responsiva.  
Permite manejar **Si5351**, **AD9850** y **ADF4351** desde cualquier navegador dentro de la misma red.

---

## ✨ Características Principales

- ⚙️ **Control Modular:** Soporte para múltiples generadores de RF (VFO Si5351, DDS AD9850, Sintetizador ADF4351) en una única interfaz.
- 🖥️ **Interfaz Web Responsiva:** Panel web en HTML, CSS y JavaScript para operación intuitiva desde cualquier navegador.
- 🔄 **Comunicación en Tiempo Real:** Uso de **WebSockets** para comunicación de baja latencia entre el ESP32 y el navegador.
- 📶 **Portal de Configuración WiFi:** Modo AP automático al primer uso para configurar fácilmente las credenciales WiFi.
- 🧾 **Pantalla OLED Local:** Muestra la IP del dispositivo, el módulo activo y el estado de conexión.
- 🔀 **Enrutamiento de Señal RF:** Control de switches por software para seleccionar el módulo activo y dirigir la salida.
- 🧰 **Herramientas de Diagnóstico:** Incluye escaneo I2C y pruebas de comunicación desde la interfaz web.

---

## 📐 Arquitectura del Software

El proyecto utiliza una arquitectura **modular y desacoplada**, separando la lógica del firmware y del frontend.

### 🧠 Firmware (ESP32 - C++)

- **`main.ino` (Orquestador):** Inicializa módulos, gestiona WiFi, WebSocket y delega comandos JSON a los manejadores.
- **Handlers de Módulos:**
  - `ad9850_handler`: Control del generador DDS AD9850.
  - `adf4351_handler`: Control del sintetizador ADF4351.
  - `vfo_handler`: Control del VFO Si5351.
- **`display_handler`:** Control centralizado de la pantalla OLED. Usa una estructura `DisplayState` actualizada por otros módulos.
- **`rf_switch_handler`:** Control de los switches RF mediante GPIO para seleccionar la ruta activa.
- **`portal_config`:** Configuración WiFi mediante portal cautivo.
- **`i2c_scanner`:** Escaneo del bus I2C para diagnóstico.

### 🌍 Frontend (HTML / CSS / JavaScript)

Aplicación **Single Page Application (SPA)** que interactúa en tiempo real con el ESP32:

- **`index.html`** – Estructura general de la interfaz.
- **`style.css`** – Estilo moderno, limpio y con estética de instrumento de laboratorio.
- **`script.js`** – Control de conexión WebSocket, envío/recepción de comandos JSON y actualización dinámica de la interfaz.

---

## 🛠️ Componentes de Hardware

| Componente | Descripción |
|-------------|-------------|
| 💻 Microcontrolador | ESP32 Dev Kit |
| ⚙️ Generadores de señal | Si5351, AD9850, ADF4351 |
| 🖥️ Pantalla | OLED SH1106G (128x64, I2C) |
| 🔀 Switches RF | Módulos controlados por GPIO |

---

## 🚀 Cómo Empezar

### 1️⃣ Flashear el Firmware
Carga el código en el ESP32 usando **Arduino IDE** o **PlatformIO**.  
Instala las librerías requeridas antes de compilar.

### 2️⃣ Primera Configuración
- Al iniciar por primera vez, el ESP32 creará una red WiFi:  
  **SSID:** `ESP32_Config`  
  **Contraseña:** `12345678`
- Conéctate desde un celular o PC.  
- Ingresa las credenciales de tu red WiFi en el **portal cautivo** y guarda.

### 3️⃣ Uso Normal
- El ESP32 se conectará a tu red local y mostrará su **IP** en la pantalla OLED.
- Desde un navegador en la misma red, abre esa IP (por ejemplo `http://192.168.1.105`).
- Carga el **panel de control web** y presiona **“Conectar”** para establecer el enlace WebSocket.
- ¡Listo! Puedes controlar los módulos de RF remotamente.

---

## 📂 Estructura del Proyecto
| Archivo / Carpeta          | Descripción                                       |
| -------------------------- | ------------------------------------------------- |
| `ad9850_handler.cpp/.h`    | Control del generador **AD9850**                  |
| `adf4351_handler.cpp/.h`   | Control del sintetizador **ADF4351**              |
| `config.h`                 | Definición de **pines** y **constantes globales** |
| `display_handler.cpp/.h`   | Control de la **pantalla OLED**                   |
| `i2c_scanner.cpp/.h`       | Escáner del **bus I2C**                           |
| `portal_config.cpp/.h`     | Portal **WiFi de configuración**                  |
| `rf_switch_handler.cpp/.h` | Control de **switches RF**                        |
| `vfo_handler.cpp/.h`       | Control del **VFO Si5351**                        |
| `main.ino`                 | **Núcleo del firmware** del ESP32                 |
| `index.html`               | **Interfaz web principal**                        |
| `script.js`                | **Lógica del frontend** (WebSocket + UI)          |
| `style.css`                | **Estilos visuales** de la interfaz web           |



---

## 📚 Librerías Necesarias

Instala las siguientes librerías en el **Arduino IDE**:

- `WiFi`, `SPI`, `WebServer`, `EEPROM`, `Wire` (incluidas con ESP32)
- `WebSockets` by Markus Sattler  
- `ArduinoJson` by Benoit Blanchon  
- `Adafruit GFX Library` by Adafruit  
- `Adafruit SH110X` by Adafruit  
- `Etherkit Si5351` (o compatible con Si5351)

---

## 🧩 Licencia

Este proyecto se distribuye bajo la licencia **MIT**, permitiendo su uso libre con atribución.

---

## 💡 Créditos


📍 Córdoba, Argentina  
📅 2025  

---

